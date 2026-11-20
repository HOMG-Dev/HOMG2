#pragma once
#include "MassEntityTypes.h"
#include "UnitFragments.generated.h"

USTRUCT()
struct FUnitIdentityFragment : public FMassFragment
{
    GENERATED_BODY()
    FGuid Id;
};

USTRUCT()
struct FUnitHealthFragment : public FMassFragment
{
    GENERATED_BODY()
    float Current = 1.f;
    float Maximum = 1.f;
};
