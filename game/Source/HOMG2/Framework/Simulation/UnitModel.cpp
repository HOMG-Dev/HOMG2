#include "Framework/Simulation/UnitModel.h"
#include "Framework/Simulation/UnitFragments.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

bool UUnitModel::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    return Super::ShouldCreateSubsystem(Outer) && World && World->IsGameWorld() && World->GetNetMode() != NM_Client;
}

void UUnitModel::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Mass = Collection.InitializeDependency<UMassEntitySubsystem>();
    check(Mass);
    const UScriptStruct* Types[] = { FUnitIdentityFragment::StaticStruct(), FUnitHealthFragment::StaticStruct() };
    Archetype = Mass->GetMutableEntityManager().CreateArchetype(Types);
}

void UUnitModel::Deinitialize()
{
    Changed.Clear();
    // The world-scoped Mass manager owns and tears down remaining entities.
    // Do not access another subsystem during world deinitialization.
    Units.Reset();
    Mass = nullptr;
    Super::Deinitialize();
}

FGuid UUnitModel::CreateUnit(float Maximum)
{
    if (GetWorld()->GetNetMode() == NM_Client || !FMath::IsFinite(Maximum) || Maximum <= 0.f) return {};
    auto& Manager = Mass->GetMutableEntityManager();
    if (Manager.IsProcessing()) return {};
    const auto Entity = Manager.CreateEntity(Archetype);
    const FGuid Id = FGuid::NewGuid();
    Manager.GetFragmentDataChecked<FUnitIdentityFragment>(Entity).Id = Id;
    auto& Health = Manager.GetFragmentDataChecked<FUnitHealthFragment>(Entity);
    Health.Current = Health.Maximum = Maximum;
    Units.Add(Id, Entity);
    return Id;
}

bool UUnitModel::SetHealth(FGuid Id, float Current)
{
    auto* Entity = Units.Find(Id);
    if (GetWorld()->GetNetMode() == NM_Client || !Entity || !FMath::IsFinite(Current)) return false;
    auto& Manager = Mass->GetMutableEntityManager();
    if (Manager.IsProcessing()) return false;
    auto& H = Manager.GetFragmentDataChecked<FUnitHealthFragment>(*Entity);
    H.Current = FMath::Clamp(Current, 0.f, H.Maximum);
    Changed.Broadcast(Id);
    return true;
}

bool UUnitModel::DestroyUnit(FGuid Id)
{
    if (GetWorld()->GetNetMode() == NM_Client || Mass->GetEntityManager().IsProcessing()) return false;
    FMassEntityHandle Entity;
    if (!Units.RemoveAndCopyValue(Id, Entity)) return false;
    Mass->GetMutableEntityManager().DestroyEntity(Entity);
    Changed.Broadcast(Id);
    return true;
}

bool UUnitModel::ReadUnit(FGuid Id, FUnitSnapshot& Out) const
{
    Out = {};
    const auto* Entity = Units.Find(Id);
    if (!Entity || GetWorld()->GetNetMode() == NM_Client) return false;
    const auto& H = Mass->GetEntityManager().GetFragmentDataChecked<FUnitHealthFragment>(*Entity);
    Out.Id = Id;
    Out.Condition = H.Current <= 0 ? EUnitCondition::Incapacitated :
        H.Current <= H.Maximum * .25f ? EUnitCondition::Critical :
        H.Current < H.Maximum ? EUnitCondition::Wounded : EUnitCondition::Healthy;
    return true;
}
