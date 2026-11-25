#include "FieldGameMode.h"

#include "M1/FieldCameraPawn.h"
#include "M1/FieldPlayerController.h"
#include "M1/UI/FieldHud.h"

AFieldGameMode::AFieldGameMode()
{
	DefaultPawnClass = AFieldCameraPawn::StaticClass();
	HUDClass = AFieldHud::StaticClass();
	PlayerControllerClass = AFieldPlayerController::StaticClass();
}
