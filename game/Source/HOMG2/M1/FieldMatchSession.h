#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FieldMatchSession.generated.h"

UENUM(BlueprintType)
enum class EFieldMatchPhase : uint8
{
	Deployment,
	Spectating,
	Settlement
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

// A value copy: changing it never changes the session.
USTRUCT(BlueprintType)
struct FFieldMatchSnapshot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FGuid SessionId;
	UPROPERTY(BlueprintReadOnly) int32 Round = 1;
	UPROPERTY(BlueprintReadOnly) EFieldMatchPhase Phase = EFieldMatchPhase::Deployment;
	UPROPERTY(BlueprintReadOnly) int32 Gold = 100;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldBattalionSnapshot> Battalions;
	UPROPERTY(BlueprintReadOnly) TArray<FFieldEquipmentSnapshot> Inventory;
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

	// Minimal operations used to exercise the state source; future gameplay commands
	// add their own validation and commit through this owner.
	bool SetGold(int32 Gold);
	bool RemoveEquipment(const FGuid& EquipmentId);

private:
	FFieldMatchSnapshot State;
	FFieldMatchChanged Changed;
};
