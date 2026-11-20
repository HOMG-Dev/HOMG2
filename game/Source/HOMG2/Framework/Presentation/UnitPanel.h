#pragma once
#include "Blueprint/UserWidget.h"
#include "Framework/UnitSnapshot.h"
#include "UnitPanel.generated.h"

class UUnitPresenter;
UCLASS()
class HOMG2_API UUnitPanel : public UUserWidget
{
    GENERATED_BODY()
public:
    // Call after AddToViewport, also when reusing a removed panel.
    UFUNCTION(BlueprintCallable) void BindPresenter(UUnitPresenter* Source);
    UFUNCTION(BlueprintNativeEvent) void RenderUnit(const FUnitSnapshot& Snapshot);
    virtual void RenderUnit_Implementation(const FUnitSnapshot& Snapshot) {}
protected:
    virtual void NativeDestruct() override;
private:
    TWeakObjectPtr<UUnitPresenter> Presenter;
    void Refresh();
};
