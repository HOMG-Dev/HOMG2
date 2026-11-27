#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldSkillSelectionTest, "HOMG2.M1.SkillSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldSkillSelectionTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UFieldMatchSession* Session = NewObject<UFieldMatchSession>(TestWorld.GetTestWorld());
	Session->InitializeFixture();
	if (!TestTrue(TEXT("Begin play"), TestWorld.BeginPlayInTestWorld())) return false;
	if (!TestTrue(TEXT("Start match clock"), Session->ConfigureAndStartClock(TestWorld.GetTestWorld(), 5.0f, 5.0f, 1.0f))) return false;
	TestTrue(TEXT("Select trench"), Session->BeginSkillSelection(EFieldSkillType::Trench).Succeeded());
	TestTrue(TEXT("Cancel selection"), Session->CancelSkillSelection().Succeeded());
	TestEqual(TEXT("Cancel leaves no preview"), Session->GetSnapshot().SkillPreviews.Num(), 0);
	TestTrue(TEXT("Select air support"), Session->BeginSkillSelection(EFieldSkillType::AirSupport).Succeeded());
	TestFalse(TEXT("Out of bounds target rejected"), Session->CommitSkillSelection(FVector(9000.0f, 0.0f, 0.0f)).Succeeded());
	TestEqual(TEXT("Rejected target leaves no preview"), Session->GetSnapshot().SkillPreviews.Num(), 0);
	TestTrue(TEXT("Valid target accepted"), Session->CommitSkillSelection(FVector(100.0f, 100.0f, 0.0f)).Succeeded());
	TestEqual(TEXT("One preview recorded"), Session->GetSnapshot().SkillPreviews.Num(), 1);
	TestFalse(TEXT("No damage is generated"), Session->GetSnapshot().Battalions[0].Condition == EFieldUnitCondition::Critical);
	return true;
}
#endif
