#include "FieldGameMode.h"

#include "Framework/Network/FrameworkPlayerController.h"
#include "GameFramework/HUD.h"
#include "M1/FieldCameraPawn.h"
#include "M1/FieldPlayerController.h"

AFieldGameMode::AFieldGameMode()
{
	DefaultPawnClass = AFieldCameraPawn::StaticClass();
	HUDClass = AHUD::StaticClass();
	PlayerControllerClass = AFieldPlayerController::StaticClass();
}
