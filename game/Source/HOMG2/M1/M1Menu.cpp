#include "M1Menu.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/WidgetPath.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "HOMG2M1Menu"

DEFINE_LOG_CATEGORY_STATIC(LogM1Menu, Log, All);

namespace
{
	constexpr float MenuButtonHeight = 48.0f;
	constexpr float MenuButtonVerticalPadding = 6.0f;
	const FName BattlefieldMap(TEXT("/Game/Maps/M1/L_M1_Field"));
}

void UM1MenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.05f));
	WidgetTree->RootWidget = Background;

	UCanvasPanel* Layout = WidgetTree->ConstructWidget<UCanvasPanel>();
	Background->SetContent(Layout);

	UVerticalBox* ButtonList = WidgetTree->ConstructWidget<UVerticalBox>();
	UCanvasPanelSlot* ButtonListSlot = Layout->AddChildToCanvas(ButtonList);
	ButtonListSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	ButtonListSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ButtonListSlot->SetPosition(FVector2D::ZeroVector);
	ButtonListSlot->SetAutoSize(true);

	StartButton = AddMenuButton(ButtonList, TEXT("StartButton"), LOCTEXT("Start", "开始游戏"));
	ExitButton = AddMenuButton(ButtonList, TEXT("ExitButton"), LOCTEXT("Exit", "退出"));
	StartButton->SetNavigationRuleExplicit(EUINavigation::Next, ExitButton);
	StartButton->SetNavigationRuleExplicit(EUINavigation::Previous, ExitButton);
	ExitButton->SetNavigationRuleExplicit(EUINavigation::Next, StartButton);
	ExitButton->SetNavigationRuleExplicit(EUINavigation::Previous, StartButton);

	TravelStatus = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TravelStatus"));
	TravelStatus->SetJustification(ETextJustify::Center);
	TravelStatus->SetAutoWrapText(true);
	TravelStatus->SetVisibility(ESlateVisibility::Collapsed);
	UCanvasPanelSlot* StatusSlot = Layout->AddChildToCanvas(TravelStatus);
	StatusSlot->SetAnchors(FAnchors(0.0f, 0.5f, 1.0f, 0.5f));
	// The stack has two equal rows, so its lower half occupies one full row.
	const float StatusTop = MenuButtonHeight + 2.0f * MenuButtonVerticalPadding + 16.0f;
	StatusSlot->SetOffsets(FMargin(16.0f, StatusTop, 16.0f, 0.0f));
	StatusSlot->SetAutoSize(true);
}

void UM1MenuWidget::SetTravelPending()
{
	StartButton->SetIsEnabled(false);
	TravelStatus->SetText(LOCTEXT("TravelPending", "正在进入战场…"));
	TravelStatus->SetColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor(218, 230, 242))));
	TravelStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UM1MenuWidget::ShowTravelFailure()
{
	StartButton->SetIsEnabled(true);
	TravelStatus->SetText(LOCTEXT("TravelFailed", "无法进入战场。请重试。"));
	TravelStatus->SetColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor(255, 210, 184))));
	TravelStatus->SetVisibility(ESlateVisibility::HitTestInvisible);
}

UButton* UM1MenuWidget::AddMenuButton(UVerticalBox* ButtonList, FName Name, const FText& Text)
{
	USizeBox* ButtonSize = WidgetTree->ConstructWidget<USizeBox>();
	ButtonSize->SetMinDesiredWidth(224.0f);
	ButtonSize->SetMinDesiredHeight(MenuButtonHeight);
	ButtonList->AddChildToVerticalBox(ButtonSize)->SetPadding(FMargin(0.0f, MenuButtonVerticalPadding));

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	ButtonSize->SetContent(Button);
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(Text);
	Label->SetJustification(ETextJustify::Center);
	Label->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
	Button->SetContent(Label);
	return Button;
}

void UM1MenuWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewFocusPath,
	const FFocusEvent& FocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewFocusPath, FocusEvent);

	const FLinearColor FocusedColor(0.3f, 0.65f, 1.0f);
	for (UButton* Button : { StartButton.Get(), ExitButton.Get() })
	{
		const TSharedPtr<SWidget> SlateButton = Button->GetCachedWidget();
		const bool bFocused = SlateButton.IsValid() && NewFocusPath.ContainsWidget(SlateButton.Get());
		Button->SetBackgroundColor(bFocused ? FocusedColor : FLinearColor::White);
	}
}

void AM1MenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	Menu = CreateWidget<UM1MenuWidget>(this);
	if (!ensure(Menu && Menu->AddToPlayerScreen()))
	{
		return;
	}

	Menu->GetStartButton()->OnClicked.AddDynamic(this, &AM1MenuPlayerController::HandleStartClicked);
	Menu->GetExitButton()->OnClicked.AddDynamic(this, &AM1MenuPlayerController::HandleExitClicked);
	if (GEngine)
	{
		TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
	}
	bShowMouseCursor = true;
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, Menu->GetStartButton(), EMouseLockMode::DoNotLock, true);
}

void AM1MenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine && TravelFailureHandle.IsValid())
	{
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
	}

	if (Menu)
	{
		Menu->GetStartButton()->OnClicked.RemoveDynamic(this, &AM1MenuPlayerController::HandleStartClicked);
		Menu->GetExitButton()->OnClicked.RemoveDynamic(this, &AM1MenuPlayerController::HandleExitClicked);
		Menu->RemoveFromParent();
		Menu = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AM1MenuPlayerController::HandleStartClicked()
{
	if (bBattlefieldTravelPending || !Menu)
	{
		return;
	}

	bBattlefieldTravelPending = true;
	Menu->SetTravelPending();
	FString MapPackage = BattlefieldMap.ToString();
	if (!GEngine || !GEngine->MakeSureMapNameIsValid(MapPackage))
	{
		UE_LOG(LogM1Menu, Warning, TEXT("Battlefield map unavailable: %s"), *BattlefieldMap.ToString());
		RestoreStartAfterTravelFailure();
		return;
	}

	UE_LOG(LogM1Menu, Display, TEXT("Starting battlefield travel: %s"), *MapPackage);
	UGameplayStatics::OpenLevel(this, FName(*MapPackage));
}

void AM1MenuPlayerController::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Reason)
{
	if (World == GetWorld() && bBattlefieldTravelPending)
	{
		UE_LOG(LogM1Menu, Warning, TEXT("Battlefield travel failed (%s): %s"), ETravelFailure::ToString(FailureType), *Reason);
		RestoreStartAfterTravelFailure();
	}
}

void AM1MenuPlayerController::RestoreStartAfterTravelFailure()
{
	bBattlefieldTravelPending = false;
	if (Menu)
	{
		Menu->ShowTravelFailure();
		Menu->GetStartButton()->SetUserFocus(this);
	}
}

void AM1MenuPlayerController::HandleExitClicked()
{
	UE_LOG(LogM1Menu, Display, TEXT("Quitting game from the M1 menu"));
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

AM1MenuGameMode::AM1MenuGameMode()
{
	PlayerControllerClass = AM1MenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	SpectatorClass = nullptr;
}

void AM1MenuGameMode::HandleStartingNewPlayer_Implementation(APlayerController* /*NewPlayer*/)
{
	// The menu runs on the local controller and does not spawn a Pawn.
}

#undef LOCTEXT_NAMESPACE
