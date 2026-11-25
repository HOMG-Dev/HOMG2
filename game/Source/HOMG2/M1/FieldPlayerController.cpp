#include "FieldPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Network/FrameworkPlayerController.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "M1/FieldCameraPawn.h"
#include "Engine/LocalPlayer.h"

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

	FieldMappingContext = NewObject<UInputMappingContext>(this, TEXT("FieldMappingContext"), RF_Transient);
	FieldMappingContext->MapKey(PanAction, EKeys::D);
	AddNegatedKey(EKeys::A, true, false);
	AddSwizzledKey(EKeys::W, false);
	AddSwizzledKey(EKeys::S, true);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Triggered, this, &ThisClass::UpdatePanInput);
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Completed, this, &ThisClass::ClearPanInput);
		EnhancedInput->BindAction(PanAction, ETriggerEvent::Canceled, this, &ThisClass::ClearPanInput);
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

void AFieldPlayerController::OnUnPossess()
{
	ClearPanInput();
	Super::OnUnPossess();
}
