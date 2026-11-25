#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/AutomationCommon.h"
#include "Components/BoxComponent.h"
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

	return true;
}

#endif
