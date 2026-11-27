#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldIntelSnapshotTest, "HOMG2.M1.IntelSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldIntelSnapshotTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UWorld* World = TestWorld.GetTestWorld();
	TStrongObjectPtr<UFieldMatchSession> Session(NewObject<UFieldMatchSession>(World));
	Session->InitializeFixture();
	if (!TestTrue(TEXT("Begin play"), TestWorld.BeginPlayInTestWorld())) return false;
	if (!TestTrue(TEXT("Start shortened clock"), Session->ConfigureAndStartClock(World, .1f, .1f, .1f))) return false;

	const FFieldMatchSnapshot Deployment = Session->GetSnapshot();
	TestEqual(TEXT("No historical snapshot in first deployment"), Deployment.HistoricalIntel.Num(), 0);
	TestEqual(TEXT("One authorized contact"), Deployment.VisibleIntel.Num(), 1);

	TestTrue(TEXT("Ready enters spectating"), Session->Ready());
	TestEqual(TEXT("Spectating keeps authorized contact"), Session->GetSnapshot().VisibleIntel.Num(), 1);
	for (int32 Frame = 0; Frame < 10 && Session->GetSnapshot().Phase != EFieldMatchPhase::Settlement; ++Frame)
	{
		TestWorld.TickTestWorld(.1f);
	}
	TestEqual(TEXT("Historical snapshot captured at settlement"), Session->GetSnapshot().HistoricalIntel.Num(), 1);
	TestEqual(TEXT("Historical location is stable"), Session->GetSnapshot().HistoricalIntel[0].Location,
		Deployment.VisibleIntel[0].Location);
	Session->StopClock();
	return true;
}
#endif
