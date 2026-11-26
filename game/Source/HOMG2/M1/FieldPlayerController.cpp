#include "FieldPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Network/FrameworkPlayerController.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "M1/FieldCameraPawn.h"
#include "Widgets/SViewport.h"

AFieldPlayerController::AFieldPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AFieldPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController())
	{
		return;
	}

	if (auto* LocalPlayer = GetLocalPlayer())
	{
		if (UGameViewportClient* Viewport = LocalPlayer->ViewportClient)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(Viewport->GetGameViewportWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);
			FlushPressedKeys();
		}

		EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
		if (EnhancedInputSubsystem.IsValid() && FieldMappingContext)
		{
			EnhancedInputSubsystem->AddMappingContext(FieldMappingContext, 0);
		}
	}
}

void AFieldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPanInput();
	ClearHeightInput();
	if (EnhancedInputSubsystem.IsValid() && FieldMappingContext)
	{
		EnhancedInputSubsystem->RemoveMappingContext(FieldMappingContext);
	}
	EnhancedInputSubsystem.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFieldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	PanAction = NewObject<UInputAction>(this, TEXT("FieldPanAction"), RF_Transient);
	PanAction->ValueType = EInputActionValueType::Axis2D;
	PanAction->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
	HeightAction = NewObject<UInputAction>(this, TEXT("FieldHeightAction"), RF_Transient);
	HeightAction->ValueType = EInputActionValueType::Axis1D;
	HeightAction->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
	ZoomAction = NewObject<UInputAction>(this, TEXT("FieldZoomAction"), RF_Transient);
	ZoomAction->ValueType = EInputActionValueType::Axis1D;

	FieldMappingContext = NewObject<UInputMappingContext>(this, TEXT("FieldMappingContext"), RF_Transient);
	FieldMappingContext->MapKey(PanAction, EKeys::D);
	AddNegatedKey(EKeys::A, true, false);
	AddSwizzledKey(EKeys::W, false);
	AddSwizzledKey(EKeys::S, true);
	FieldMappingContext->MapKey(HeightAction, EKeys::E);
	{
		FEnhancedActionKeyMapping& Mapping = FieldMappingContext->MapKey(HeightAction, EKeys::Q);
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(FieldMappingContext);
		Mapping.Modifiers.Add(Negate);
	}
	FieldMappingContext->MapKey(ZoomAction, EKeys::MouseWheelAxis);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Triggered, this, &ThisClass::UpdatePanInput);
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Completed, this, &ThisClass::ClearPanInput);
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Canceled, this, &ThisClass::ClearPanInput);
		EnhancedInput->BindAction(HeightAction, ETriggerEvent::Triggered, this, &ThisClass::UpdateHeightInput);
		EnhancedInput->BindAction(HeightAction, ETriggerEvent::Completed, this, &ThisClass::ClearHeightInput);
		EnhancedInput->BindAction(HeightAction, ETriggerEvent::Canceled, this, &ThisClass::ClearHeightInput);
		EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ThisClass::ApplyZoomInput);
	}
}

void AFieldPlayerController::AddNegatedKey(const FKey Key, const bool bNegateX, const bool bNegateY)
{
	FEnhancedActionKeyMapping& Mapping = FieldMappingContext->MapKey(PanAction, Key);
	UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(FieldMappingContext);
	Negate->bX = bNegateX;
	Negate->bY = bNegateY;
	Mapping.Modifiers.Add(Negate);
}

void AFieldPlayerController::AddSwizzledKey(const FKey Key, const bool bNegate)
{
	FEnhancedActionKeyMapping& Mapping = FieldMappingContext->MapKey(PanAction, Key);
	if (bNegate)
	{
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(FieldMappingContext);
		Negate->bY = false;
		Negate->bZ = false;
		Mapping.Modifiers.Add(Negate);
	}
	UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(FieldMappingContext);
	Mapping.Modifiers.Add(Swizzle);
}

void AFieldPlayerController::UpdatePanInput(const FInputActionValue& Value)
{
	if (AFieldCameraPawn* FieldPawn = Cast<AFieldCameraPawn>(GetPawn()))
	{
		FieldPawn->SetPanInput(Value.Get<FVector2D>());
	}
}

void AFieldPlayerController::ClearPanInput()
{
	if (AFieldCameraPawn* FieldPawn = Cast<AFieldCameraPawn>(GetPawn()))
	{
		FieldPawn->SetPanInput(FVector2D::ZeroVector);
	}
}

void AFieldPlayerController::UpdateHeightInput(const FInputActionValue& Value)
{
	if (AFieldCameraPawn* FieldPawn = Cast<AFieldCameraPawn>(GetPawn()))
	{
		FieldPawn->SetHeightInput(Value.Get<float>());
	}
}

void AFieldPlayerController::ClearHeightInput()
{
	if (AFieldCameraPawn* FieldPawn = Cast<AFieldCameraPawn>(GetPawn()))
	{
		FieldPawn->SetHeightInput(0.0f);
	}
}

void AFieldPlayerController::ApplyZoomInput(const FInputActionValue& Value)
{
	if (AFieldCameraPawn* FieldPawn = Cast<AFieldCameraPawn>(GetPawn()))
	{
		FieldPawn->ApplyZoomInput(Value.Get<float>());
	}
}

void AFieldPlayerController::OnUnPossess()
{
	ClearPanInput();
	ClearHeightInput();
	Super::OnUnPossess();
}
