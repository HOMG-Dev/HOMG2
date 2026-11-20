#pragma once
#include "Components/ActorComponent.h"
#include "Framework/UnitSnapshot.h"
#include "UnitFeed.generated.h"

// Add to a player-owned controller only. Entries double as the server's visibility whitelist.
UCLASS()
class HOMG2_API UUnitFeed : public UActorComponent
{
    GENERATED_BODY()
public:
    UUnitFeed();
    bool Grant(FGuid Id);
    void Revoke(FGuid Id);
    const TArray<FUnitSnapshot>& GetVisible() const { return Visible; }
    DECLARE_EVENT(UUnitFeed, FUpdated);
    FUpdated& OnUpdated() { return Updated; }
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(ReplicatedUsing=OnRep_Visible) TArray<FUnitSnapshot> Visible;
    UFUNCTION() void OnRep_Visible();
    void Refresh(FGuid Id);
    void Publish();
    int32 Index(FGuid Id) const;
    FUpdated Updated;
};
