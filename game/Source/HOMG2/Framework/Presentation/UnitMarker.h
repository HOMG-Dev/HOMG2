#pragma once
#include "Components/TextRenderComponent.h"
#include "Framework/UnitSnapshot.h"
#include "UnitMarker.generated.h"
class UUnitPresenter;

// Local visual only. Bind to a stable ID, not to whichever unit is selected.
UCLASS(Blueprintable, ClassGroup=(HOMG2), meta=(BlueprintSpawnableComponent))
class HOMG2_API UUnitMarker : public UTextRenderComponent
{
    GENERATED_BODY()
public:
    UUnitMarker();
    UFUNCTION(BlueprintCallable) void BindUnit(UUnitPresenter* Source, FGuid Id);
    UFUNCTION(BlueprintNativeEvent) void RenderUnit(const FUnitSnapshot& Snapshot);
    virtual void RenderUnit_Implementation(const FUnitSnapshot& Snapshot);
protected:
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TWeakObjectPtr<UUnitPresenter> Presenter;
    FGuid Unit;
    void Refresh();
};
