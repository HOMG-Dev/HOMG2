#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FieldCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

UCLASS()
class HOMG2_API AFieldCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AFieldCameraPawn();

	virtual void Tick(float DeltaSeconds) override;

	void SetPanInput(FVector2D Input);
	void SetHeightInput(float Input);
	void ApplyZoomInput(float Input);

protected:
	virtual void BeginPlay() override;

private:
	bool FindFieldBounds();
	double ClampHeight(double Height) const;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Movement", meta = (ClampMin = "0.0"))
	float PanSpeed = 5200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Movement", meta = (ClampMin = "0.0"))
	float BoundaryInset = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "0.0"))
	float ZoomStep = 900.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float MinArmLength = 4000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float MaxArmLength = 26000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Height", meta = (ClampMin = "0.0"))
	float HeightSpeed = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Height", meta = (ClampMin = "0.0"))
	float MinGroundClearance = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Height", meta = (ClampMin = "0.0"))
	float MaxGroundClearance = 12000.0f;

	FVector2D PanInput = FVector2D::ZeroVector;
	float HeightInput = 0.0f;
	FVector2D FieldCenter = FVector2D::ZeroVector;
	FVector2D FieldExtent = FVector2D::ZeroVector;
	double FieldGroundHeight = 0.0;
	bool bHasFieldBounds = false;
};
