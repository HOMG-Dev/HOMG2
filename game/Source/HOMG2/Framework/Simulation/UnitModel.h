#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityManager.h"
#include "Framework/UnitSnapshot.h"
#include "UnitModel.generated.h"

class UMassEntitySubsystem;

UCLASS()
class HOMG2_API UUnitModel : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Authority only, game thread, outside Mass processing. Invalid GUID/false = rejected.
    FGuid CreateUnit(float Maximum);
    bool SetHealth(FGuid Id, float Current);
    bool DestroyUnit(FGuid Id);
    bool ReadUnit(FGuid Id, FUnitSnapshot& Out) const;
    DECLARE_EVENT_OneParam(UUnitModel, FChanged, FGuid);
    FChanged& OnChanged() { return Changed; }
private:
    FChanged Changed;
    UPROPERTY() TObjectPtr<UMassEntitySubsystem> Mass;
    FMassArchetypeHandle Archetype;
    TMap<FGuid, FMassEntityHandle> Units;
};
