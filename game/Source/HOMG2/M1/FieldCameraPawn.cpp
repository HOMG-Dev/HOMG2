#include "FieldCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AFieldCameraPawn::AFieldCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(SceneRoot);
	CameraArm->SetRelativeRotation(FRotator(-70.0f, 0.0f, 0.0f));
	CameraArm->TargetArmLength = 13500.0f;
	CameraArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->SetFieldOfView(55.0f);
}

void AFieldCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	bHasFieldBounds = FindFieldBounds();
}

bool AFieldCameraPawn::FindFieldBounds()
{
	TArray<AActor*> GroundActors;
	UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("M1.Field.Ground")), GroundActors);
	if (GroundActors.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Field camera requires an actor tagged M1.Field.Ground."));
		return false;
	}

	FVector BoundsExtent;
	GroundActors[0]->GetActorBounds(false, FieldCenter, BoundsExtent);
	FieldExtent = FVector2D(BoundsExtent.X, BoundsExtent.Y);
	FieldCenter.Z = GetActorLocation().Z;
	return FieldExtent.X > BoundaryInset && FieldExtent.Y > BoundaryInset;
}

void AFieldCameraPawn::SetPanInput(const FVector2D Input)
{
	PanInput = Input.GetClampedToMaxSize(1.0f);
}

void AFieldCameraPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bHasFieldBounds || DeltaSeconds <= 0.0f || PanInput.IsNearlyZero())
	{
		return;
	}

	FVector PanRight = Camera->GetRightVector();
	PanRight.Z = 0.0f;
	PanRight.Normalize();
	FVector PanUp = Camera->GetUpVector();
	PanUp.Z = 0.0f;
	PanUp.Normalize();
	FVector PanDirection = PanRight * PanInput.X + PanUp * PanInput.Y;
	PanDirection = PanDirection.GetClampedToMaxSize(1.0f);

	FVector Location = GetActorLocation() + PanDirection * PanSpeed * DeltaSeconds;
	Location.X = FMath::Clamp(Location.X,
		FieldCenter.X - FieldExtent.X + BoundaryInset,
		FieldCenter.X + FieldExtent.X - BoundaryInset);
	Location.Y = FMath::Clamp(Location.Y,
		FieldCenter.Y - FieldExtent.Y + BoundaryInset,
		FieldCenter.Y + FieldExtent.Y - BoundaryInset);
	SetActorLocation(Location);
}
