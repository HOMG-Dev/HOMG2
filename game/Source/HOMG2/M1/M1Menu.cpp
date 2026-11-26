#include "M1Menu.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Layout/WidgetPath.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "HOMG2M1Menu"

DEFINE_LOG_CATEGORY_STATIC(LogM1Menu, Log, All);

void UM1MenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	Background->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.05f));
	Background->SetHorizontalAlignment(HAlign_Center);
	Background->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Background;

	UVerticalBox* ButtonList = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->SetContent(ButtonList);

	StartButton = AddMenuButton(ButtonList, TEXT("StartButton"), LOCTEXT("Start", "开始游戏"));
	ExitButton = AddMenuButton(ButtonList, TEXT("ExitButton"), LOCTEXT("Exit", "退出"));
	StartButton->SetNavigationRuleExplicit(EUINavigation::Next, ExitButton);
	StartButton->SetNavigationRuleExplicit(EUINavigation::Previous, ExitButton);
	ExitButton->SetNavigationRuleExplicit(EUINavigation::Next, StartButton);
	ExitButton->SetNavigationRuleExplicit(EUINavigation::Previous, StartButton);
}

UButton* UM1MenuWidget::AddMenuButton(UVerticalBox* ButtonList, FName Name, const FText& Text)
{
	USizeBox* ButtonSize = WidgetTree->ConstructWidget<USizeBox>();
	ButtonSize->SetMinDesiredWidth(224.0f);
	ButtonSize->SetMinDesiredHeight(48.0f);
	ButtonList->AddChildToVerticalBox(ButtonSize)->SetPadding(FMargin(0.0f, 6.0f));

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
	bShowMouseCursor = true;
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, Menu->GetStartButton(), EMouseLockMode::DoNotLock, true);
}

void AM1MenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
	UE_LOG(LogM1Menu, Display, TEXT("Start selected"));
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
