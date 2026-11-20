#include "Framework/Network/UnitFeed.h"
#include "Framework/Simulation/UnitModel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

UUnitFeed::UUnitFeed() { SetIsReplicatedByDefault(true); }

void UUnitFeed::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner()->HasAuthority())
        if (auto* Model = GetWorld()->GetSubsystem<UUnitModel>())
            Model->OnChanged().AddUObject(this, &ThisClass::Refresh);
}

void UUnitFeed::EndPlay(const EEndPlayReason::Type Reason)
{
    if (auto* Model = GetWorld()->GetSubsystem<UUnitModel>()) Model->OnChanged().RemoveAll(this);
    Updated.Clear();
    Super::EndPlay(Reason);
}

void UUnitFeed::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UUnitFeed, Visible, COND_OwnerOnly);
}

int32 UUnitFeed::Index(FGuid Id) const
{
    return Visible.IndexOfByPredicate([Id](const FUnitSnapshot& S){ return S.Id == Id; });
}

bool UUnitFeed::Grant(FGuid Id)
{
    if (!GetOwner()->IsA<APlayerController>() || !GetOwner()->HasAuthority()) return false;
    auto* Model = GetWorld()->GetSubsystem<UUnitModel>();
    FUnitSnapshot S;
    if (!Model || !Model->ReadUnit(Id, S)) return false;
    if (Index(Id) == INDEX_NONE) { Visible.Add(S); Publish(); }
    return true;
}

void UUnitFeed::Revoke(FGuid Id)
{
    if (!GetOwner()->HasAuthority()) return;
    const int32 I = Index(Id);
    if (I != INDEX_NONE) { Visible.RemoveAtSwap(I); Publish(); }
}

void UUnitFeed::Refresh(FGuid Id)
{
    const int32 I = Index(Id);
    if (I == INDEX_NONE) return;
    FUnitSnapshot S;
    if (!GetWorld()->GetSubsystem<UUnitModel>()->ReadUnit(Id, S)) { Revoke(Id); return; }
    if (Visible[I].Condition != S.Condition) { Visible[I] = S; Publish(); }
}

void UUnitFeed::Publish()
{
    GetOwner()->ForceNetUpdate();
    OnRep_Visible(); // The local listen host never receives its own property replication.
}

void UUnitFeed::OnRep_Visible() { Updated.Broadcast(); }
