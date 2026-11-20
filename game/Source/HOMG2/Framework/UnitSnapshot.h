#pragma once
#include "CoreMinimal.h"
#include "UnitSnapshot.generated.h"

UENUM(BlueprintType)
enum class EUnitCondition : uint8 { Healthy, Wounded, Critical, Incapacitated };

// Public read model. An invalid ID means no visible unit.
USTRUCT(BlueprintType)
struct HOMG2_API FUnitSnapshot
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FGuid Id;
    UPROPERTY(BlueprintReadOnly) EUnitCondition Condition = EUnitCondition::Healthy;
};
