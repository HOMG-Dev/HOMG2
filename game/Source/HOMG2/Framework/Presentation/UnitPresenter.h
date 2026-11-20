#pragma once
#include "Framework/UnitSnapshot.h"
#include "UObject/Object.h"
#include "UnitPresenter.generated.h"

class UUnitFeed;
UCLASS(BlueprintType)
class HOMG2_API UUnitPresenter : public UObject
{
    GENERATED_BODY()
public:
    void Attach(UUnitFeed* InFeed);
    // Local selection only. An invalid ID clears selection.
    UFUNCTION(BlueprintCallable) void SelectUnit(FGuid Id);
    UFUNCTION(BlueprintPure) FUnitSnapshot GetSelected() const;
    UFUNCTION(BlueprintPure) FUnitSnapshot GetUnit(FGuid Id) const;
    DECLARE_EVENT(UUnitPresenter, FChanged);
    FChanged& OnChanged() { return Changed; }
private:
    void Refresh();
    TWeakObjectPtr<UUnitFeed> Feed;
    FGuid Selected;
    FChanged Changed;
};
