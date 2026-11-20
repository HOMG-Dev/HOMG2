#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "Framework/Simulation/UnitModel.h"
#include "Framework/Network/FrameworkPlayerController.h"
#include "Framework/Network/UnitFeed.h"
#include "Framework/Presentation/UnitPresenter.h"
#include "Framework/Presentation/UnitMarker.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnitFrameworkTest, "HOMG2.Framework.UnitLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnitFrameworkTest::RunTest(const FString&)
{
    FTestWorldWrapper World;
    if (!TestTrue(TEXT("Game world"), World.CreateTestWorld(EWorldType::Game))) return false;
    UWorld* W = World.GetTestWorld();
    UUnitModel* M = W->GetSubsystem<UUnitModel>();
    if (!TestNotNull(TEXT("Mass model"), M)) return false;
    TestFalse(TEXT("Reject invalid maximum"), M->CreateUnit(0).IsValid());
    const FGuid Id = M->CreateUnit(100);
    if (!TestTrue(TEXT("Stable id"), Id.IsValid())) return false;
    AFrameworkPlayerController* PC = W->SpawnActor<AFrameworkPlayerController>();
    if (!TestNotNull(TEXT("Controller"), PC)) return false;
    if (!TestTrue(TEXT("Begin play"), World.BeginPlayInTestWorld())) return false;
    auto* Feed = PC->GetUnitFeed();
    TestTrue(TEXT("Grant once"), Feed->Grant(Id));
    Feed->Grant(Id);
    TestEqual(TEXT("No duplicate"), Feed->GetVisible().Num(), 1);
    auto* Presenter = NewObject<UUnitPresenter>(PC);
    Presenter->Attach(Feed); // Snapshot predates the Presenter and its View.
    Presenter->SelectUnit(Id);
    auto* Marker = NewObject<UUnitMarker>(PC);
    Marker->BindUnit(Presenter, Id);
    TestTrue(TEXT("Late view receives snapshot"), Marker->IsVisible());
    auto CheckState = [&](float Value, EUnitCondition Expected)
    {
        TestTrue(TEXT("Write health"), M->SetHealth(Id, Value));
        TestEqual(TEXT("Selected state"), Presenter->GetSelected().Condition, Expected);
        TestEqual(TEXT("Scene view rendered same state"), Marker->Text.ToString(), UEnum::GetDisplayValueAsText(Expected).ToString());
    };
    CheckState(175, EUnitCondition::Healthy);
    CheckState(70, EUnitCondition::Wounded);
    CheckState(25, EUnitCondition::Critical);
    CheckState(-5, EUnitCondition::Incapacitated);
    Feed->Revoke(Id);
    TestFalse(TEXT("Selection cleared on revoke"), Presenter->GetSelected().Id.IsValid());
    TestFalse(TEXT("World marker hidden on revoke"), Marker->IsVisible());
    Feed->Grant(Id);
    TestTrue(TEXT("Destroy"), M->DestroyUnit(Id));
    TestTrue(TEXT("Destroyed id removed"), Feed->GetVisible().IsEmpty());
    TestFalse(TEXT("Stale id cannot mutate"), M->SetHealth(Id, 10));
    TestFalse(TEXT("Stale id cannot be granted"), Feed->Grant(Id));
    Marker->BindUnit(nullptr, {});
    Presenter->Attach(nullptr);
    return true;
}
#endif
