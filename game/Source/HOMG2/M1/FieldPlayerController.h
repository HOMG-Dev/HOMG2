#pragma once

#include "CoreMinimal.h"
#include "Framework/Network/FrameworkPlayerController.h"
#include "FieldPlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;

UCLASS()
class HOMG2_API AFieldPlayerController : public AFrameworkPlayerController
{
	GENERATED_BODY()

public:
	AFieldPlayerController();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnUnPossess() override;

private:
	void AddNegatedKey(FKey Key, bool bNegateX, bool bNegateY);
	void AddSwizzledKey(FKey Key, bool bNegate);
	void UpdatePanInput(const struct FInputActionValue& Value);
	void ClearPanInput();
	void UpdateHeightInput(const struct FInputActionValue& Value);
	void ClearHeightInput();
	void ApplyZoomInput(const struct FInputActionValue& Value);

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PanAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> HeightAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> FieldMappingContext;

	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> EnhancedInputSubsystem;
};
