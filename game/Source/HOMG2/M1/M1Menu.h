#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "M1Menu.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class HOMG2_API UM1MenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UButton* GetStartButton() const { return StartButton; }
	UButton* GetExitButton() const { return ExitButton; }
	void SetTravelPending();
	void ShowTravelFailure();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewFocusPath,
		const FFocusEvent& FocusEvent) override;

private:
	UButton* AddMenuButton(UVerticalBox* ButtonList, FName Name, const FText& Text);

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TravelStatus;
};

UCLASS()
class HOMG2_API AM1MenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleExitClicked();

	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Reason);
	void RestoreStartAfterTravelFailure();

	UPROPERTY(Transient)
	TObjectPtr<UM1MenuWidget> Menu;
	FDelegateHandle TravelFailureHandle;
	bool bBattlefieldTravelPending = false;
};

UCLASS()
class HOMG2_API AM1MenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AM1MenuGameMode();

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};
