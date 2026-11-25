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

protected:
	virtual void BeginPlay() override;

private:
	bool FindFieldBounds();

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

	FVector2D PanInput = FVector2D::ZeroVector;
	FVector FieldCenter = FVector::ZeroVector;
	FVector2D FieldExtent = FVector2D::ZeroVector;
	bool bHasFieldBounds = false;
};
