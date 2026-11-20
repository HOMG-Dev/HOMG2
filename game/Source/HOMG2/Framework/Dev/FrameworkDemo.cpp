#include "Framework/Dev/FrameworkDemo.h"
#include "Framework/Network/UnitFeed.h"
#include "Framework/Presentation/UnitMarker.h"
#include "Framework/Presentation/UnitPresenter.h"
#include "Framework/Simulation/UnitModel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "GameFramework/SpectatorPawn.h"
#include "Engine/World.h"

AFrameworkDemoMode::AFrameworkDemoMode()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    PlayerControllerClass = AFrameworkDemoController::StaticClass();
    DefaultPawnClass = ASpectatorPawn::StaticClass();
#endif
}

TSharedRef<SWidget> UFrameworkDemoPanel::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        Label = WidgetTree->ConstructWidget<UTextBlock>();
        WidgetTree->RootWidget = Label;
    }
    return Super::RebuildWidget();
}

void UFrameworkDemoPanel::RenderUnit_Implementation(const FUnitSnapshot& Snapshot)
{
    if (Label) Label->SetText(Snapshot.Id.IsValid() ? UEnum::GetDisplayValueAsText(Snapshot.Condition) :
        NSLOCTEXT("HOMG2", "DemoEmpty", "No unit selected"));
}

void AFrameworkDemoController::BeginPlay()
{
    Super::BeginPlay();
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (HasAuthority())
    {
        DemoUnit = GetWorld()->GetSubsystem<UUnitModel>()->CreateUnit(100);
        GetUnitFeed()->Grant(DemoUnit);
    }
    if (GetUnitPresenter())
    {
        Panel = CreateWidget<UFrameworkDemoPanel>(this);
        Panel->AddToPlayerScreen();
        Panel->BindPresenter(GetUnitPresenter());
        Marker = NewObject<UUnitMarker>(this);
        AddInstanceComponent(Marker);
        Marker->RegisterComponent();
        GetUnitFeed()->OnUpdated().AddUObject(this, &ThisClass::ReceiveFeed);
        ReceiveFeed();
    }
#endif
}

void AFrameworkDemoController::ReceiveFeed()
{
    if (GetUnitFeed()->GetVisible().IsEmpty()) return;
    const FGuid Id = GetUnitFeed()->GetVisible()[0].Id;
    GetUnitPresenter()->SelectUnit(Id);
    Marker->BindUnit(GetUnitPresenter(), Id);
    FVector Location;
    FRotator Rotation;
    GetPlayerViewPoint(Location, Rotation);
    Marker->SetWorldLocationAndRotation(Location + Rotation.Vector()*300, (-Rotation.Vector()).Rotation());
    UE_LOG(LogTemp, Display, TEXT("FrameworkDemo visible=%d id=%s state=%s"), GetUnitFeed()->GetVisible().Num(),
        *Id.ToString(), *UEnum::GetValueAsString(GetUnitPresenter()->GetSelected().Condition));
}

void AFrameworkDemoController::DemoSelect()
{
    if (GetUnitPresenter() && !GetUnitFeed()->GetVisible().IsEmpty())
        GetUnitPresenter()->SelectUnit(GetUnitFeed()->GetVisible()[0].Id);
}

void AFrameworkDemoController::DemoDamage() { ServerDemoDamage(); }

void AFrameworkDemoController::ServerDemoDamage_Implementation()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // Fixed test action, no caller-supplied target or attribute value.
    if (DemoUnit.IsValid()) GetWorld()->GetSubsystem<UUnitModel>()->SetHealth(DemoUnit, 20);
#endif
}

void AFrameworkDemoController::EndPlay(const EEndPlayReason::Type Reason)
{
    GetUnitFeed()->OnUpdated().RemoveAll(this);
    if (Panel) Panel->RemoveFromParent();
    if (HasAuthority() && DemoUnit.IsValid())
        if (auto* M = GetWorld()->GetSubsystem<UUnitModel>()) M->DestroyUnit(DemoUnit);
    Super::EndPlay(Reason);
}
