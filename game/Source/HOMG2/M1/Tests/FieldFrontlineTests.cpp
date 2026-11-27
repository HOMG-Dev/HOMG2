#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldFrontlineTest, "HOMG2.M1.FrontlineSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldFrontlineTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UWorld* World = TestWorld.GetTestWorld();
	UFieldMatchSession* Session = NewObject<UFieldMatchSession>(World);
	Session->InitializeFixture();
	const FFieldMatchSnapshot Snapshot = Session->GetSnapshot();
	TestTrue(TEXT("Frontline has multiple points"), Snapshot.Frontline.Num() >= 2);
	TestFalse(TEXT("Enemy frontline is not exposed"), Snapshot.Frontline.IsEmpty());
	return true;
}
#endif
