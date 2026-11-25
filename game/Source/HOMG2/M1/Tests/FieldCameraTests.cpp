#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/AutomationCommon.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "M1/FieldCameraPawn.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldCameraMovementTest, "HOMG2.M1.FieldCameraMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldCameraMovementTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create game world"), TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	AActor* Ground = World->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Spawn field bounds"), Ground))
	{
		return false;
	}

	UBoxComponent* Bounds = NewObject<UBoxComponent>(Ground, TEXT("FieldBounds"));
	Ground->AddInstanceComponent(Bounds);
	Ground->SetRootComponent(Bounds);
	Bounds->SetBoxExtent(FVector(6000.0f, 3600.0f, 100.0f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->RegisterComponent();
	Ground->Tags.Add(FName(TEXT("M1.Field.Ground")));

	AFieldCameraPawn* Pawn = World->SpawnActor<AFieldCameraPawn>();
	if (!TestNotNull(TEXT("Spawn camera pawn"), Pawn) || !TestWorld.BeginPlayInTestWorld())
	{
		return false;
	}
	TestEqual(TEXT("Camera pawn starts above ground clearance"), Pawn->GetActorLocation().Z, 400.0);

	auto MoveForHalfSecond = [Pawn](const FVector2D Input)
	{
		Pawn->SetPanInput(Input);
		Pawn->Tick(0.5f);
	};
	MoveForHalfSecond(FVector2D(1.0f, 0.0f));
	TestEqual(TEXT("D pans right in screen space"), Pawn->GetActorLocation().Y, 2600.0);
	MoveForHalfSecond(FVector2D(-1.0f, 0.0f));
	TestEqual(TEXT("A pans left in screen space"), Pawn->GetActorLocation().Y, 0.0);
	MoveForHalfSecond(FVector2D(0.0f, 1.0f));
	TestEqual(TEXT("W pans up in screen space"), Pawn->GetActorLocation().X, 2600.0);
	MoveForHalfSecond(FVector2D(0.0f, -1.0f));
	TestEqual(TEXT("S pans down in screen space"), Pawn->GetActorLocation().X, 0.0);

	Pawn->SetPanInput(FVector2D(0.0f, 1.0f));
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Top edge is clamped"), Pawn->GetActorLocation().X, 5500.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding north cannot leave the field"), Pawn->GetActorLocation().X, 5500.0);
	MoveForHalfSecond(FVector2D(0.0f, -1.0f));
	TestTrue(TEXT("South input moves away from the north edge"), Pawn->GetActorLocation().X < 5500.0f);

	Pawn->SetActorLocation(FVector::ZeroVector);
	Pawn->SetPanInput(FVector2D(0.0f, -1.0f));
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Bottom edge is clamped"), Pawn->GetActorLocation().X, -5500.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding south cannot leave the field"), Pawn->GetActorLocation().X, -5500.0);
	MoveForHalfSecond(FVector2D(0.0f, 1.0f));
	TestTrue(TEXT("North input moves away from the south edge"), Pawn->GetActorLocation().X > -5500.0f);

	Pawn->SetActorLocation(FVector::ZeroVector);
	Pawn->SetPanInput(FVector2D(1.0f, 0.0f));
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Right edge is clamped"), Pawn->GetActorLocation().Y, 3100.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding east cannot leave the field"), Pawn->GetActorLocation().Y, 3100.0);
	MoveForHalfSecond(FVector2D(-1.0f, 0.0f));
	TestTrue(TEXT("West input moves away from the east edge"), Pawn->GetActorLocation().Y < 3100.0f);

	Pawn->SetActorLocation(FVector::ZeroVector);
	Pawn->SetPanInput(FVector2D(-1.0f, 0.0f));
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Left edge is clamped"), Pawn->GetActorLocation().Y, -3100.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding west cannot leave the field"), Pawn->GetActorLocation().Y, -3100.0);
	MoveForHalfSecond(FVector2D(1.0f, 0.0f));
	TestTrue(TEXT("East input moves away from the west edge"), Pawn->GetActorLocation().Y > -3100.0f);

	Pawn->SetPanInput(FVector2D::ZeroVector);
	Pawn->SetActorLocation(FVector(0.0, 0.0, 400.0));
	const USpringArmComponent* CameraArm = Pawn->FindComponentByClass<USpringArmComponent>();
	if (!TestNotNull(TEXT("Find camera spring arm"), CameraArm))
	{
		return false;
	}
	Pawn->ApplyZoomInput(1.0f);
	TestEqual(TEXT("Wheel input zooms in without changing height"), CameraArm->TargetArmLength, 12600.0f);
	TestEqual(TEXT("Zoom input leaves pawn height unchanged"), Pawn->GetActorLocation().Z, 400.0);
	Pawn->ApplyZoomInput(-1.0f);
	TestEqual(TEXT("Reverse wheel input restores zoom"), CameraArm->TargetArmLength, 13500.0f);
	Pawn->ApplyZoomInput(100.0f);
	TestEqual(TEXT("Zoom-in stops at local-view minimum"), CameraArm->TargetArmLength, 4000.0f);
	Pawn->ApplyZoomInput(-1.0f);
	TestTrue(TEXT("Reverse wheel input escapes zoom minimum"), CameraArm->TargetArmLength > 4000.0f);
	Pawn->ApplyZoomInput(-100.0f);
	TestEqual(TEXT("Zoom-out stops at global-view maximum"), CameraArm->TargetArmLength, 26000.0f);
	Pawn->ApplyZoomInput(1.0f);
	TestTrue(TEXT("Reverse wheel input escapes zoom maximum"), CameraArm->TargetArmLength < 26000.0f);

	const float ArmLengthBeforeHeight = CameraArm->TargetArmLength;
	Pawn->SetHeightInput(-1.0f);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Height stops at ground clearance minimum"), Pawn->GetActorLocation().Z, 400.0);
	Pawn->SetHeightInput(1.0f);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Height reaches configured global maximum"), Pawn->GetActorLocation().Z, 12100.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding height cannot exceed maximum"), Pawn->GetActorLocation().Z, 12100.0);
	Pawn->SetHeightInput(-1.0f);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Reverse height input escapes maximum"), Pawn->GetActorLocation().Z, 400.0);
	Pawn->Tick(100.0f);
	TestEqual(TEXT("Holding height cannot pass below ground"), Pawn->GetActorLocation().Z, 400.0);
	Pawn->SetHeightInput(1.0f);
	Pawn->Tick(0.5f);
	TestEqual(TEXT("Reverse height input escapes minimum"), Pawn->GetActorLocation(), FVector(0.0, 0.0, 1300.0));
	Pawn->SetHeightInput(0.0f);
	Pawn->Tick(1.0f);
	TestEqual(TEXT("Released height input leaves the camera stationary"), Pawn->GetActorLocation().Z, 1300.0);
	TestEqual(TEXT("Height input leaves arm length unchanged"), CameraArm->TargetArmLength, ArmLengthBeforeHeight);

	return true;
}

#endif
