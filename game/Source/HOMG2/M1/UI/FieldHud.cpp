#include "FieldHud.h"

#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "M1/FieldGameMode.h"
#include "M1/FieldMatchSession.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "M1FieldHud"

namespace
{
const FLinearColor Panel(0.019f, 0.031f, 0.046f, 0.97f);
const FLinearColor Row(0.038f, 0.058f, 0.077f);
const FLinearColor Selection(0.046f, 0.118f, 0.120f);
const FLinearColor Ink(0.91f, 0.95f, 0.98f);
const FLinearColor Muted(0.54f, 0.66f, 0.73f);
const FLinearColor Accent(0.35f, 0.88f, 0.77f);

TSharedRef<STextBlock> Text(const FText& Label, int32 Size, const FLinearColor& Color = Ink, bool bBold = false)
{
	return SNew(STextBlock)
		.Text(Label)
		.Font(FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), Size))
		.ColorAndOpacity(Color)
		.OverflowPolicy(ETextOverflowPolicy::Ellipsis);
}

TSharedRef<SBorder> Surface(TSharedRef<SWidget> Content, const FLinearColor& Color, FMargin Padding)
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Color)
		.Padding(Padding)
		[Content];
}

TSharedRef<SButton> FixtureButton(TSharedRef<SWidget> Content, const FLinearColor& Color)
{
	return SNew(SButton)
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.ContentPadding(0)
		.IsFocusable(false)
		.OnClicked_Lambda([] { return FReply::Handled(); })
		[Surface(Content, Color, FMargin(12, 8))];
}

TSharedRef<SWidget> Stat(const FText& Label, const FText& Value, const FLinearColor& Color = Ink)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Text(Label, 10, Muted, true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 0)[Text(Value, 15, Color, true)];
}

FText PhaseLabel(const EFieldMatchPhase Phase)
{
	switch (Phase)
	{
	case EFieldMatchPhase::Spectating: return LOCTEXT("Spectating", "Spectating");
	case EFieldMatchPhase::Settlement: return LOCTEXT("Settlement", "Settlement");
	default: return LOCTEXT("Deployment", "Deployment");
	}
}

FText SkillPreviewLabel(const EFieldSkillType Type)
{
	switch (Type)
	{
	case EFieldSkillType::Trench: return LOCTEXT("PreviewTrench", "TEST PREVIEW: TRENCH");
	case EFieldSkillType::Wire: return LOCTEXT("PreviewWire", "TEST PREVIEW: WIRE");
	case EFieldSkillType::AntiPersonnelMine: return LOCTEXT("PreviewAntiPersonnelMine", "TEST PREVIEW: AP MINE");
	case EFieldSkillType::AntiTankMine: return LOCTEXT("PreviewAntiTankMine", "TEST PREVIEW: AT MINE");
	case EFieldSkillType::AirSupport: return LOCTEXT("PreviewAirSupport", "TEST PREVIEW: AIR SUPPORT");
	default: return LOCTEXT("PreviewUnknown", "TEST PREVIEW");
	}
}

TSharedRef<SWidget> StatusBar(const FFieldMatchSnapshot& Snapshot)
{
	return Surface(SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
		[Text(LOCTEXT("Sector", "Training grounds"), 20, Ink, true)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 26, 0)
		[Stat(LOCTEXT("Round", "ROUND"), FText::AsNumber(Snapshot.Round))]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 26, 0)
		[Stat(LOCTEXT("Phase", "PHASE"), PhaseLabel(Snapshot.Phase), Accent)]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 26, 0)
		[Stat(LOCTEXT("Time", "TIME LEFT"), FText::FromString(FString::Printf(TEXT("%02d:%02d"), Snapshot.SecondsRemaining / 60, Snapshot.SecondsRemaining % 60)))]
		+ SHorizontalBox::Slot().AutoWidth()
		[Stat(LOCTEXT("Gold", "GOLD"), FText::AsNumber(Snapshot.Gold))], Panel, FMargin(16, 10));
}

TSharedRef<SWidget> RosterButton(const FText& Name, const FText& Strength, bool bSelected = false)
{
	return FixtureButton(SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Text(Name, 13, Ink, true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)[Text(Strength, 11, bSelected ? Accent : Muted)],
		bSelected ? Selection : Row);
}

TSharedRef<SWidget> BattalionList()
{
	return Surface(SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
		[Text(LOCTEXT("Battalions", "Battalions"), 17, Ink, true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
		[RosterButton(LOCTEXT("Vanguard", "Vanguard"), LOCTEXT("VanguardStrength", "8 / 9 members"), true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
		[RosterButton(LOCTEXT("Watch", "Watch Company"), LOCTEXT("WatchStrength", "6 / 9 members"))]
		+ SVerticalBox::Slot().AutoHeight()
		[RosterButton(LOCTEXT("Reserve", "Reserve"), LOCTEXT("ReserveStrength", "4 / 9 members"))], Panel, FMargin(12));
}

TSharedRef<SWidget> MemberCard(const FText& Role, const FText& Status)
{
	return Surface(SNew(SBox).HeightOverride(81).VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[Text(Role, 13, Ink, true)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)[Text(Status, 11, Muted)]
		], Row, FMargin(12, 0));
}

FText ConditionLabel(const EFieldUnitCondition Condition)
{
	switch (Condition)
	{
	case EFieldUnitCondition::Critical: return LOCTEXT("Critical", "Critical");
	case EFieldUnitCondition::Wounded: return LOCTEXT("Wounded", "Wounded");
	default: return LOCTEXT("Healthy", "Healthy");
	}
}

TSharedRef<SWidget> BattalionDetails(const FFieldMatchSnapshot& Snapshot)
{
	if (!Snapshot.bDetailsAuthorized)
	{
		return Surface(Text(LOCTEXT("DetailsRevoked", "Unit details unavailable"), 14, Muted, true), Panel, FMargin(12));
	}
	const FFieldBattalionSnapshot* Battalion = Snapshot.Battalions.IsEmpty() ? nullptr : &Snapshot.Battalions[0];
	const FText Deployed = Battalion ? ConditionLabel(Battalion->Condition) : LOCTEXT("MemberDeployed", "In formation");
	const FText Roles[] = {
		LOCTEXT("Commander", "Field commander"), LOCTEXT("Scout", "Scout"), LOCTEXT("Signal", "Signaller"),
		LOCTEXT("Guard", "Guard"), LOCTEXT("Engineer", "Engineer"), LOCTEXT("Medic", "Medic"),
		LOCTEXT("FirstRifle", "Rifleman A"), LOCTEXT("SecondRifle", "Rifleman B"),
	};
	TSharedRef<SScrollBox> Members = SNew(SScrollBox)
		.ScrollBarVisibility(EVisibility::Visible)
		.AnimateWheelScrolling(false)
		.ConsumeMouseWheel(EConsumeMouseWheel::Always);
	for (const FText& Role : Roles)
	{
		Members->AddSlot().Padding(0, 0, 5, 6)[MemberCard(Role, Deployed)];
	}
	Members->AddSlot().Padding(0, 0, 5, 0)
		[MemberCard(LOCTEXT("Courier", "Courier"), LOCTEXT("EnRouteMember", "En route"))];

	return Surface(SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Text(LOCTEXT("BattalionDetails", "BATTALION DETAILS"), 10, Accent, true)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 12)[Text(Battalion ? Battalion->Name : LOCTEXT("Vanguard", "Vanguard"), 20, Ink, true)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			Surface(SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1)[Stat(LOCTEXT("Members", "PRESENT"), FText::AsNumber(Battalion ? Battalion->Members.Num() : 0))]
				+ SHorizontalBox::Slot().FillWidth(1)[Stat(LOCTEXT("Transit", "EN ROUTE"), FText::AsNumber(0))]
				+ SHorizontalBox::Slot().FillWidth(1)[Stat(LOCTEXT("Capacity", "CAPACITY"), FText::AsNumber(9))],
				Selection, FMargin(9, 8))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 14, 0, 8)[Text(LOCTEXT("Roster", "Roster"), 13, Ink, true)]
		+ SVerticalBox::Slot().FillHeight(1)[Members], Panel, FMargin(12));
}

TSharedRef<SWidget> ActionButton(const FText& Label, const FLinearColor& Color, const FOnClicked& Handler)
{
	return SNew(SButton)
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.ContentPadding(0)
		.OnClicked(Handler)
		[Surface(Text(Label, 11, Ink, true), Color, FMargin(10, 8))];
}

TSharedRef<SWidget> ActionBar(const FFieldMatchSnapshot& Snapshot, const FOnClicked& ReadyHandler,
	const FOnClicked& TrenchHandler, const FOnClicked& WireHandler, const FOnClicked& AntiPersonnelMineHandler,
	const FOnClicked& AntiTankMineHandler, const FOnClicked& AirSupportHandler)
{
	TSharedRef<SHorizontalBox> RowWidget = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
		[Text(Snapshot.Phase == EFieldMatchPhase::Deployment ? LOCTEXT("DeploymentControls", "DEPLOYMENT CONTROLS") : LOCTEXT("ReadOnly", "READ-ONLY OBSERVATION"), 10, Muted, true)];
	if (Snapshot.Phase == EFieldMatchPhase::Deployment)
	{
		RowWidget->AddSlot().AutoWidth().Padding(0, 0, 4, 0)[ActionButton(LOCTEXT("Trench", "Trench"), Row, TrenchHandler)];
		RowWidget->AddSlot().AutoWidth().Padding(0, 0, 4, 0)[ActionButton(LOCTEXT("Wire", "Wire"), Row, WireHandler)];
		RowWidget->AddSlot().AutoWidth().Padding(0, 0, 4, 0)[ActionButton(LOCTEXT("AntiPersonnelMine", "AP mine"), Row, AntiPersonnelMineHandler)];
		RowWidget->AddSlot().AutoWidth().Padding(0, 0, 4, 0)[ActionButton(LOCTEXT("AntiTankMine", "AT mine"), Row, AntiTankMineHandler)];
		RowWidget->AddSlot().AutoWidth().Padding(0, 0, 4, 0)[ActionButton(LOCTEXT("AirSupport", "Air support"), Row, AirSupportHandler)];
		RowWidget->AddSlot().AutoWidth()[ActionButton(LOCTEXT("Ready", "Ready"), Selection, ReadyHandler)];
	}
	return Surface(RowWidget, Panel, FMargin(12, 7));
}

TSharedRef<SWidget> BuildLayout(const FFieldMatchSnapshot& Snapshot, const FOnClicked& ReadyHandler,
	const FOnClicked& TrenchHandler, const FOnClicked& WireHandler, const FOnClicked& AntiPersonnelMineHandler,
	const FOnClicked& AntiTankMineHandler, const FOnClicked& AirSupportHandler)
{
	return SNew(SOverlay).Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot().Padding(12)
		[
			SNew(SVerticalBox).Visibility(EVisibility::SelfHitTestInvisible)
			+ SVerticalBox::Slot().AutoHeight()[StatusBar(Snapshot)]
			+ SVerticalBox::Slot().FillHeight(1).Padding(0, 8)
			[
				SNew(SHorizontalBox).Visibility(EVisibility::SelfHitTestInvisible)
				+ SHorizontalBox::Slot().FillWidth(0.19f)[BattalionList()]
				+ SHorizontalBox::Slot().FillWidth(0.57f)[SNew(SSpacer).Visibility(EVisibility::HitTestInvisible)]
				+ SHorizontalBox::Slot().FillWidth(0.24f)[BattalionDetails(Snapshot)]
			]
			+ SVerticalBox::Slot().AutoHeight()[ActionBar(Snapshot, ReadyHandler, TrenchHandler, WireHandler,
				AntiPersonnelMineHandler, AntiTankMineHandler, AirSupportHandler)]
		]
		+ SOverlay::Slot()
		[
			SNew(SOverlay).Visibility(EVisibility::HitTestInvisible)
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0, 0, 0, 75)
			[
				Surface(Text(LOCTEXT("MapControls", "WASD   Pan      /      Wheel   Zoom      /      Q E   Height"), 11, Ink),
					Panel, FMargin(12, 6))
			]
		];
}
}

AFieldHud::AFieldHud()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFieldHud::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* Controller = GetOwningPlayerController();
	ULocalPlayer* Player = Controller ? Controller->GetLocalPlayer() : nullptr;
	UGameViewportClient* Viewport = Player ? Cast<UGameViewportClient>(Player->GetViewportClient()) : nullptr;
	if (Viewport)
	{
		LocalPlayer = Player;
		ViewportClient = Viewport;
		if (AFieldGameMode* GameMode = GetWorld()->GetAuthGameMode<AFieldGameMode>())
		{
			MatchSession = GameMode->GetMatchSession();
			if (MatchSession.IsValid())
			{
				MatchChangedHandle = MatchSession->OnChanged().AddUObject(this, &ThisClass::HandleMatchChanged);
			}
		}

		const FFieldMatchSnapshot Snapshot = MatchSession.IsValid() ? MatchSession->GetSnapshot() : FFieldMatchSnapshot{};
		Layout = BuildLayout(Snapshot, FOnClicked::CreateUObject(this, &ThisClass::HandleReadyClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleTrenchClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleWireClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAntiPersonnelMineClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAntiTankMineClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAirSupportClicked));
		Viewport->AddViewportWidgetForPlayer(Player, Layout.ToSharedRef(), 0);
	}
}

void AFieldHud::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MatchSession.IsValid())
	{
		MatchSession->OnChanged().Remove(MatchChangedHandle);
	}
	RemoveLayout();
	MatchSession.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFieldHud::DrawHUD()
{
	Super::DrawHUD();
	APlayerController* Controller = GetOwningPlayerController();
	if (!Canvas || !Controller || !MatchSession.IsValid())
	{
		return;
	}

	const FFieldMatchSnapshot Snapshot = MatchSession->GetSnapshot();
	const TArray<FFieldIntelSnapshot>& Markers = Snapshot.Phase == EFieldMatchPhase::Spectating
		? Snapshot.VisibleIntel : Snapshot.HistoricalIntel;
	const FLinearColor MarkerColor = Snapshot.Phase == EFieldMatchPhase::Spectating
		? Accent : FLinearColor(0.55f, 0.62f, 0.68f, 0.75f);
	if (Snapshot.Phase == EFieldMatchPhase::Deployment)
	{
		for (int32 Index = 1; Index < Snapshot.Frontline.Num(); ++Index)
		{
			FVector2D Previous;
			FVector2D Current;
			if (Controller->ProjectWorldLocationToScreen(Snapshot.Frontline[Index - 1], Previous) &&
				Controller->ProjectWorldLocationToScreen(Snapshot.Frontline[Index], Current))
			{
				DrawLine(Previous.X, Previous.Y, Current.X, Current.Y,
					FLinearColor(0.45f, 0.65f, 0.78f, 0.8f), 3.0f);
			}
		}
	}
	for (const FFieldIntelSnapshot& Marker : Markers)
	{
		FVector2D Screen;
		if (Controller->ProjectWorldLocationToScreen(Marker.Location, Screen))
		{
			DrawText(Marker.Label.ToString(), MarkerColor, Screen.X, Screen.Y, nullptr, 1.1f);
		}
	}
	if (Snapshot.bDetailsAuthorized)
	{
		for (const FFieldBattalionSnapshot& Battalion : Snapshot.Battalions)
		{
			FVector2D Screen;
			if (Controller->ProjectWorldLocationToScreen(Battalion.Location, Screen))
			{
				DrawText(ConditionLabel(Battalion.Condition).ToString(),
					Battalion.Condition == EFieldUnitCondition::Critical ? FLinearColor(1.0f, 0.45f, 0.35f) : Accent,
					Screen.X, Screen.Y + 22.0f, nullptr, 1.0f);
			}
		}
	}
	for (const FFieldSkillPreview& Preview : Snapshot.SkillPreviews)
	{
		FVector2D Screen;
		if (Controller->ProjectWorldLocationToScreen(Preview.Location, Screen))
		{
			const bool bAirSupport = Preview.Type == EFieldSkillType::AirSupport;
			const FLinearColor PreviewColor = bAirSupport
				? FLinearColor(1.0f, 0.62f, 0.25f, 0.95f)
				: Accent;
			const float Left = Screen.X - 24.0f;
			const float Top = Screen.Y - 24.0f;
			const float Right = Screen.X + 24.0f;
			const float Bottom = Screen.Y + 24.0f;
			DrawLine(Left, Top, Right, Top, PreviewColor, 2.0f);
			DrawLine(Right, Top, Right, Bottom, PreviewColor, 2.0f);
			DrawLine(Right, Bottom, Left, Bottom, PreviewColor, 2.0f);
			DrawLine(Left, Bottom, Left, Top, PreviewColor, 2.0f);
			DrawLine(Screen.X - 7.0f, Screen.Y, Screen.X + 7.0f, Screen.Y, PreviewColor, 1.0f);
			DrawLine(Screen.X, Screen.Y - 7.0f, Screen.X, Screen.Y + 7.0f, PreviewColor, 1.0f);
			DrawText(SkillPreviewLabel(Preview.Type).ToString(), PreviewColor, Left, Bottom + 5.0f, nullptr, 0.85f);
		}
	}
}

void AFieldHud::HandleMatchChanged()
{
	if (MatchSession.IsValid() && ViewportClient.IsValid() && LocalPlayer.IsValid())
	{
		RemoveLayout();
		Layout = BuildLayout(MatchSession->GetSnapshot(), FOnClicked::CreateUObject(this, &ThisClass::HandleReadyClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleTrenchClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleWireClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAntiPersonnelMineClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAntiTankMineClicked),
			FOnClicked::CreateUObject(this, &ThisClass::HandleAirSupportClicked));
		ViewportClient->AddViewportWidgetForPlayer(LocalPlayer.Get(), Layout.ToSharedRef(), 0);
	}
}

FReply AFieldHud::HandleReadyClicked()
{
	if (MatchSession.IsValid())
	{
		MatchSession->Ready();
	}
	return FReply::Handled();
}

FReply AFieldHud::HandleTrenchClicked()
{
	if (MatchSession.IsValid()) MatchSession->BeginSkillSelection(EFieldSkillType::Trench);
	return FReply::Handled();
}

FReply AFieldHud::HandleWireClicked()
{
	if (MatchSession.IsValid()) MatchSession->BeginSkillSelection(EFieldSkillType::Wire);
	return FReply::Handled();
}

FReply AFieldHud::HandleAntiPersonnelMineClicked()
{
	if (MatchSession.IsValid()) MatchSession->BeginSkillSelection(EFieldSkillType::AntiPersonnelMine);
	return FReply::Handled();
}

FReply AFieldHud::HandleAntiTankMineClicked()
{
	if (MatchSession.IsValid()) MatchSession->BeginSkillSelection(EFieldSkillType::AntiTankMine);
	return FReply::Handled();
}

FReply AFieldHud::HandleAirSupportClicked()
{
	if (MatchSession.IsValid()) MatchSession->BeginSkillSelection(EFieldSkillType::AirSupport);
	return FReply::Handled();
}

void AFieldHud::RemoveLayout()
{
	if (Layout.IsValid() && ViewportClient.IsValid() && LocalPlayer.IsValid())
	{
		ViewportClient->RemoveViewportWidgetForPlayer(LocalPlayer.Get(), Layout.ToSharedRef());
	}
	Layout.Reset();
}

#undef LOCTEXT_NAMESPACE
