#pragma once
#include "GameFramework/PlayerController.h"
#include "FrameworkPlayerController.generated.h"
class UUnitFeed;
class UUnitPresenter;
UCLASS()
class HOMG2_API AFrameworkPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AFrameworkPlayerController();
    UFUNCTION(BlueprintPure) UUnitPresenter* GetUnitPresenter() const { return Presenter; }
    UUnitFeed* GetUnitFeed() const { return Feed; }
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UUnitFeed> Feed;
    UPROPERTY(Transient) TObjectPtr<UUnitPresenter> Presenter;
};
