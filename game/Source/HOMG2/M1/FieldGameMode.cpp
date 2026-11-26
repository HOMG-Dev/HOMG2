#include "FieldGameMode.h"

#include "M1/FieldCameraPawn.h"
#include "M1/FieldPlayerController.h"
#include "M1/FieldMatchSession.h"
#include "M1/UI/FieldHud.h"

AFieldGameMode::AFieldGameMode()
{
	DefaultPawnClass = AFieldCameraPawn::StaticClass();
	HUDClass = AFieldHud::StaticClass();
	PlayerControllerClass = AFieldPlayerController::StaticClass();
}

void AFieldGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// M1 is a local fixture, not a replicated multiplayer state source.
	if (GetNetMode() == NM_Standalone && !MatchSession)
	{
		MatchSession = NewObject<UFieldMatchSession>(this);
		MatchSession->InitializeFixture();
	}
}


void AFieldGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (MatchSession)
	{
		MatchSession->ConfigureAndStartClock(GetWorld());
	}
}

void AFieldGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MatchSession)
	{
		MatchSession->StopClock();
	}
	Super::EndPlay(EndPlayReason);
}
