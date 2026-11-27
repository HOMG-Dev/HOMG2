#include "M1/FieldMatchSession.h"
#include "Engine/World.h"

void UFieldMatchSession::InitializeFixture()
{
	if (State.SessionId.IsValid())
	{
		return;
	}
	State = FFieldMatchSnapshot{};
	State.SessionId = FGuid::NewGuid();
	FFieldBattalionSnapshot Battalion;
	Battalion.Id = FGuid::NewGuid();
	Battalion.Name = NSLOCTEXT("FieldMatch", "Vanguard", "Vanguard");
	Battalion.Location = FVector(-2800.0f, 0.0f, 0.0f);
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FFieldMemberSnapshot Member;
		Member.Id = FGuid::NewGuid();
		Battalion.Members.Add(Member);
	}
	State.Battalions.Add(Battalion);
	FFieldEquipmentSnapshot Equipment;
	Equipment.Id = FGuid::NewGuid();
	Equipment.Kind = TEXT("TestEquipment");
	State.Inventory.Add(Equipment);
	State.VisibleIntel.Add({FVector(900.0f, 450.0f, 30.0f), NSLOCTEXT("FieldMatch", "Contact", "Contact")});
	State.HistoricalIntel.Reset();
	State.Frontline = {
		FVector(-4200.0f, -850.0f, 35.0f), FVector(-1800.0f, -250.0f, 35.0f),
		FVector(400.0f, -500.0f, 35.0f), FVector(2900.0f, 250.0f, 35.0f)
	};
}

bool UFieldMatchSession::SetTestWound()
{
	if (State.Battalions.IsEmpty() || State.Battalions[0].Members.IsEmpty())
	{
		return false;
	}
	State.Battalions[0].Condition = EFieldUnitCondition::Critical;
	State.Battalions[0].Members[0].Condition = EFieldUnitCondition::Critical;
	Changed.Broadcast();
	return true;
}

FFieldCommandResult UFieldMatchSession::BeginSkillSelection(const EFieldSkillType Type)
{
	FFieldCommandResult Result = ValidateDeploymentCommand();
	if (!Result.Succeeded()) return Result;
	if (Type == EFieldSkillType::None)
	{
		Result.Code = EFieldCommandCode::InvalidTarget;
		Result.Reason = NSLOCTEXT("FieldMatch", "InvalidSkill", "技能类型无效。");
		return Result;
	}
	PendingSkill = Type;
	Changed.Broadcast();
	return Result;
}

FFieldCommandResult UFieldMatchSession::CancelSkillSelection()
{
	FFieldCommandResult Result;
	if (!IsSkillSelectionActive())
	{
		Result.Code = EFieldCommandCode::NoSkillSelection;
		Result.Reason = NSLOCTEXT("FieldMatch", "NoSkillSelection", "当前没有待选技能。");
		return Result;
	}
	PendingSkill = EFieldSkillType::None;
	Changed.Broadcast();
	return Result;
}

FFieldCommandResult UFieldMatchSession::CommitSkillSelection(const FVector Location)
{
	FFieldCommandResult Result = ValidateDeploymentCommand();
	if (!Result.Succeeded()) return Result;
	if (!IsSkillSelectionActive())
	{
		Result.Code = EFieldCommandCode::NoSkillSelection;
		Result.Reason = NSLOCTEXT("FieldMatch", "NoSkillSelection", "当前没有待选技能。");
		return Result;
	}
	if (!FMath::IsFinite(Location.X) || !FMath::IsFinite(Location.Y) || Location.X < -5000.0f || Location.X > 5000.0f ||
		Location.Y < -3000.0f || Location.Y > 3000.0f)
	{
		Result.Code = EFieldCommandCode::InvalidTarget;
		Result.Reason = NSLOCTEXT("FieldMatch", "InvalidSkillTarget", "技能目标超出战场范围。");
		return Result;
	}
	State.SkillPreviews.Add({PendingSkill, Location});
	PendingSkill = EFieldSkillType::None;
	Changed.Broadcast();
	return Result;
}

bool UFieldMatchSession::ToggleFrontlineSample()
{
	State.FrontlineSample = State.FrontlineSample == 0 ? 1 : 0;
	State.Frontline = State.FrontlineSample == 0
		? TArray<FVector>{FVector(-4200.0f, -850.0f, 35.0f), FVector(-1800.0f, -250.0f, 35.0f), FVector(400.0f, -500.0f, 35.0f), FVector(2900.0f, 250.0f, 35.0f)}
		: TArray<FVector>{FVector(-4200.0f, -500.0f, 35.0f), FVector(-1600.0f, 150.0f, 35.0f), FVector(600.0f, 50.0f, 35.0f), FVector(2900.0f, 650.0f, 35.0f)};
	Changed.Broadcast();
	return true;
}

#if WITH_DEV_AUTOMATION_TESTS
bool UFieldMatchSession::SetGold(int32 Gold)
{
	if (Gold < 0 || Gold == State.Gold)
	{
		return false;
	}
	State.Gold = Gold;
	Changed.Broadcast();
	return true;
}

bool UFieldMatchSession::RemoveEquipment(const FGuid& EquipmentId)
{
	const int32 Index = State.Inventory.IndexOfByPredicate([&](const FFieldEquipmentSnapshot& Item)
	{
		return Item.Id == EquipmentId;
	});
	if (Index == INDEX_NONE)
	{
		return false;
	}
	State.Inventory.RemoveAt(Index);
	Changed.Broadcast();
	return true;
}

#endif

bool UFieldMatchSession::ConfigureAndStartClock(UWorld* World, float DeploymentSeconds,
	float SpectatingSeconds, float SettlementSeconds)
{
	if (!IsValid(World) || !State.SessionId.IsValid() || ClockWorld.IsValid() ||
		!FMath::IsFinite(DeploymentSeconds) || !FMath::IsFinite(SpectatingSeconds) || !FMath::IsFinite(SettlementSeconds) ||
		DeploymentSeconds <= 0 || SpectatingSeconds <= 0 || SettlementSeconds <= 0 ||
		DeploymentSeconds + SpectatingSeconds + SettlementSeconds > 120.0f)
	{
		return false;
	}
	ClockWorld = World;
	DeploymentDuration = DeploymentSeconds;
	SpectatingDuration = SpectatingSeconds;
	SettlementDuration = SettlementSeconds;
	PhaseDeadline = World->GetTimeSeconds() + DurationForPhase();
	State.SecondsRemaining = FMath::CeilToInt(DurationForPhase());
	World->GetTimerManager().SetTimer(ClockTimer, this, &UFieldMatchSession::UpdateClock, 0.1f, true);
	Changed.Broadcast();
	return true;
}

void UFieldMatchSession::StopClock()
{
	if (UWorld* World = ClockWorld.Get())
	{
		World->GetTimerManager().ClearTimer(ClockTimer);
	}
	ClockWorld.Reset();
}

float UFieldMatchSession::DurationForPhase() const
{
	switch (State.Phase)
	{
	case EFieldMatchPhase::Deployment: return DeploymentDuration;
	case EFieldMatchPhase::Spectating: return SpectatingDuration;
	default: return SettlementDuration;
	}
}

void UFieldMatchSession::UpdateClock()
{
	UWorld* World = ClockWorld.Get();
	if (!World)
	{
		return;
	}
	const double Remaining = PhaseDeadline - World->GetTimeSeconds();
	if (Remaining <= 0.0f)
	{
		AdvancePhase();
		return;
	}
	const int32 Seconds = FMath::CeilToInt(Remaining);
	if (Seconds != State.SecondsRemaining)
	{
		State.SecondsRemaining = Seconds;
		Changed.Broadcast();
	}
}

void UFieldMatchSession::AdvancePhase()
{
	UWorld* World = ClockWorld.Get();
	if (!World)
	{
		return;
	}
	switch (State.Phase)
	{
	case EFieldMatchPhase::Deployment: State.Phase = EFieldMatchPhase::Spectating; break;
	case EFieldMatchPhase::Spectating:
		State.HistoricalIntel = State.VisibleIntel;
		State.Phase = EFieldMatchPhase::Settlement;
		break;
	case EFieldMatchPhase::Settlement: State.Phase = EFieldMatchPhase::Deployment; ++State.Round; break;
	}
	PhaseDeadline = World->GetTimeSeconds() + DurationForPhase();
	State.SecondsRemaining = FMath::CeilToInt(DurationForPhase());
	Changed.Broadcast();
}

bool UFieldMatchSession::Ready()
{
	if (!ClockWorld.IsValid() || State.Phase != EFieldMatchPhase::Deployment)
	{
		return false;
	}
	AdvancePhase();
	return true;
}

FFieldCommandResult UFieldMatchSession::ValidateDeploymentCommand() const
{
	FFieldCommandResult Result;
	if (!ClockWorld.IsValid())
	{
		Result.Code = EFieldCommandCode::NoActiveMatch;
		Result.Reason = NSLOCTEXT("FieldMatch", "NoActiveMatch", "当前没有进行中的对局。");
	}
	else if (State.Phase != EFieldMatchPhase::Deployment)
	{
		Result.Code = EFieldCommandCode::WrongPhase;
		Result.Reason = NSLOCTEXT("FieldMatch", "WrongPhase", "只能在部署阶段执行此操作。");
	}
	return Result;
}

FFieldCommandResult UFieldMatchSession::SpendTestGold(int32 Amount)
{
	FFieldCommandResult Result = ValidateDeploymentCommand();
	if (!Result.Succeeded())
	{
		return Result;
	}
	if (Amount <= 0)
	{
		Result.Code = EFieldCommandCode::InvalidAmount;
		Result.Reason = NSLOCTEXT("FieldMatch", "InvalidAmount", "消耗金额必须大于零。");
		return Result;
	}
	if (Amount > State.Gold)
	{
		Result.Code = EFieldCommandCode::InsufficientGold;
		Result.Reason = NSLOCTEXT("FieldMatch", "InsufficientGold", "金币不足。");
		return Result;
	}
	State.Gold -= Amount;
	Changed.Broadcast();
	return Result;
}
