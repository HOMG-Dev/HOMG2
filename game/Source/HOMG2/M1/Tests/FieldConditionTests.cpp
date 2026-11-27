#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldConditionTest, "HOMG2.M1.ConditionSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldConditionTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UFieldMatchSession* Session = NewObject<UFieldMatchSession>(TestWorld.GetTestWorld());
	Session->InitializeFixture();
	TestTrue(TEXT("Test wound updates shared snapshot"), Session->SetTestWound());
	const FFieldMatchSnapshot Snapshot = Session->GetSnapshot();
	TestEqual(TEXT("Battalion exposes condition state"), Snapshot.Battalions[0].Condition, EFieldUnitCondition::Critical);
	TestEqual(TEXT("Member exposes condition state"), Snapshot.Battalions[0].Members[0].Condition, EFieldUnitCondition::Critical);
	return true;
}
#endif
