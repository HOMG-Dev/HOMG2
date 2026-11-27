#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TimerManager.h"
#include "FieldMatchSession.generated.h"

UENUM(BlueprintType)
enum class EFieldMatchPhase : uint8
{
	Deployment,
	Spectating,
	Settlement
};

UENUM(BlueprintType)
enum class EFieldCommandCode : uint8
{
	Success,
	NoActiveMatch,
	WrongPhase,
	InvalidAmount,
	InsufficientGold
};

USTRUCT(BlueprintType)
struct FFieldCommandResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) EFieldCommandCode Code = EFieldCommandCode::Success;
	UPROPERTY(BlueprintReadOnly) FText Reason;
	bool Succeeded() const { return Code == EFieldCommandCode::Success; }
};

USTRUCT(BlueprintType)
struct FFieldMemberSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid Id;
};

USTRUCT(BlueprintType)
struct FFieldBattalionSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid Id;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldMemberSnapshot> Members;
};

USTRUCT(BlueprintType)
struct FFieldEquipmentSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid Id;
	UPROPERTY(BlueprintReadOnly) FName Kind;
};

USTRUCT(BlueprintType)
struct FFieldIntelSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FText Label;
};

// A value copy: changing it never changes the session.
USTRUCT(BlueprintType)
struct FFieldMatchSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid SessionId;
	UPROPERTY(BlueprintReadOnly) int32 Round = 1;
	UPROPERTY(BlueprintReadOnly) EFieldMatchPhase Phase = EFieldMatchPhase::Deployment;
	UPROPERTY(BlueprintReadOnly) int32 Gold = 100;
	UPROPERTY(BlueprintReadOnly) int32 SecondsRemaining = 45;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldBattalionSnapshot> Battalions;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldEquipmentSnapshot> Inventory;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldIntelSnapshot> VisibleIntel;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldIntelSnapshot> HistoricalIntel;
	UPROPERTY(BlueprintReadOnly) TArray<FVector> Frontline;
};

DECLARE_MULTICAST_DELEGATE(FFieldMatchChanged);

// Owned by the field GameMode; never persisted across battlefield loads.
UCLASS()
class HOMG2_API UFieldMatchSession : public UObject
{
	GENERATED_BODY()
public:
	void InitializeFixture();

	UFUNCTION(BlueprintPure, Category="M1|Match")
	FFieldMatchSnapshot GetSnapshot() const { return State; }

	FFieldMatchChanged& OnChanged() { return Changed; }
	// Phase durations must be positive and total at most 120 seconds.
	bool ConfigureAndStartClock(UWorld* World, float DeploymentSeconds = 45.0f,
		float SpectatingSeconds = 45.0f, float SettlementSeconds = 5.0f);
	void StopClock();
	UFUNCTION(BlueprintCallable, Category="M1|Match")
	bool Ready();

	// Fixture-only mutations for the D1 isolation test, unavailable to runtime commands.
#if WITH_DEV_AUTOMATION_TESTS
	bool SetGold(int32 Gold);
	bool RemoveEquipment(const FGuid& EquipmentId);
#endif

	// Shared phase gate for deployment operations; check before any mutation.
	FFieldCommandResult ValidateDeploymentCommand() const;
	UFUNCTION(BlueprintCallable, Category="M1|Match")
	FFieldCommandResult SpendTestGold(int32 Amount);

private:
	void UpdateClock();
	void AdvancePhase();
	float DurationForPhase() const;
	TWeakObjectPtr<UWorld> ClockWorld;
	FTimerHandle ClockTimer;
	double PhaseDeadline = 0.0;
	float DeploymentDuration = 45.0f;
	float SpectatingDuration = 45.0f;
	float SettlementDuration = 5.0f;
	FFieldMatchSnapshot State;
	FFieldMatchChanged Changed;
};
