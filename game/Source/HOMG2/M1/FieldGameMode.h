#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FieldGameMode.generated.h"

class UFieldMatchSession;

UCLASS()
class HOMG2_API AFieldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFieldGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category="M1|Match")
	UFieldMatchSession* GetMatchSession() const { return MatchSession; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UFieldMatchSession> MatchSession;
};
