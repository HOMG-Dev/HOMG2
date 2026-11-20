#include "Framework/Network/FrameworkPlayerController.h"
#include "Framework/Network/UnitFeed.h"
#include "Framework/Presentation/UnitPresenter.h"

AFrameworkPlayerController::AFrameworkPlayerController()
{
    Feed = CreateDefaultSubobject<UUnitFeed>(TEXT("UnitFeed"));
}

void AFrameworkPlayerController::BeginPlay()
{
    // Make the Presenter available to the controller Blueprint's ReceiveBeginPlay.
    if (IsLocalController() && GetNetMode() != NM_DedicatedServer)
    {
        Presenter = NewObject<UUnitPresenter>(this);
        Presenter->Attach(Feed);
    }
    Super::BeginPlay();
}

void AFrameworkPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Presenter) Presenter->Attach(nullptr);
    Presenter = nullptr;
    Super::EndPlay(Reason);
}
