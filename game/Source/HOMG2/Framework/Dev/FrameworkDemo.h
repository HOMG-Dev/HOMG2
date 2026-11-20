#pragma once
#include "GameFramework/GameModeBase.h"
#include "Framework/Network/FrameworkPlayerController.h"
#include "Framework/Presentation/UnitPanel.h"
#include "FrameworkDemo.generated.h"
class UUnitMarker;
class UTextBlock;

UCLASS()
class UFrameworkDemoPanel : public UUnitPanel
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void RenderUnit_Implementation(const FUnitSnapshot& Snapshot) override;
private:
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Label;
};

UCLASS()
class AFrameworkDemoController : public AFrameworkPlayerController
{
    GENERATED_BODY()
public:
    UFUNCTION(Exec) void DemoDamage();
    UFUNCTION(Exec) void DemoSelect();
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UFUNCTION(Server, Reliable) void ServerDemoDamage();
    void ReceiveFeed();
    FGuid DemoUnit;
    UPROPERTY(Transient) TObjectPtr<UFrameworkDemoPanel> Panel;
    UPROPERTY(Transient) TObjectPtr<UUnitMarker> Marker;
};

UCLASS()
class AFrameworkDemoMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AFrameworkDemoMode();
};
