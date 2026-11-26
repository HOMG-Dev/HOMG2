#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldMatchClockTest, "HOMG2.M1.MatchClock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldMatchClockTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create game world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UWorld* World = TestWorld.GetTestWorld();
	TStrongObjectPtr<UFieldMatchSession> Session(NewObject<UFieldMatchSession>(World));
	Session->InitializeFixture();
	if (!TestTrue(TEXT("Begin play"), TestWorld.BeginPlayInTestWorld())) return false;
	TestFalse(TEXT("Reject invalid durations"), Session->ConfigureAndStartClock(World, 0, 1, 1));
	TestFalse(TEXT("Reject a cycle over 120 seconds"), Session->ConfigureAndStartClock(World, 60, 60, 1));
	TestTrue(TEXT("Start shortened clock"), Session->ConfigureAndStartClock(World, .5f, .5f, .5f));
	TestFalse(TEXT("Cannot start another clock"), Session->ConfigureAndStartClock(World));
	int32 PhaseChanges = 0;
	EFieldMatchPhase Previous = Session->GetSnapshot().Phase;
	auto Handle = Session->OnChanged().AddLambda([&] {
		EFieldMatchPhase Current = Session->GetSnapshot().Phase;
		if (Current != Previous) { ++PhaseChanges; Previous = Current; }
	});
	auto TickUntil = [&](EFieldMatchPhase Expected) {
		for (int32 Frame = 0; Frame < 30 && Session->GetSnapshot().Phase != Expected; ++Frame)
		{
			if (!TestWorld.TickTestWorld(.1f)) return false;
		}
		return Session->GetSnapshot().Phase == Expected;
	};
	TestTrue(TEXT("Timeout starts spectating"), TickUntil(EFieldMatchPhase::Spectating));
	TestFalse(TEXT("Ready after timeout is rejected"), Session->Ready());
	TestEqual(TEXT("Timeout transitions only once"), PhaseChanges, 1);
	TestTrue(TEXT("Spectating reaches settlement"), TickUntil(EFieldMatchPhase::Settlement));
	TestEqual(TEXT("Only one transition to settlement"), PhaseChanges, 2);
	TestTrue(TEXT("Settlement reaches deployment"), TickUntil(EFieldMatchPhase::Deployment));
	TestEqual(TEXT("Second round"), Session->GetSnapshot().Round, 2);
	TestTrue(TEXT("Ready advances immediately"), Session->Ready());
	TestFalse(TEXT("Duplicate ready rejected"), Session->Ready());
	TestEqual(TEXT("Each phase transition delivered once"), PhaseChanges, 4);
	TestTrue(TEXT("Tick next frame"), TestWorld.TickTestWorld(.1f));
	TestTrue(TEXT("Old timeout cannot advance early"), Session->GetSnapshot().Phase == EFieldMatchPhase::Spectating);
	Session->StopClock();
	const int32 BeforeStop = PhaseChanges;
	for (int32 Frame = 0; Frame < 15; ++Frame) TestWorld.TickTestWorld(.1f);
	TestEqual(TEXT("Stop removes timer"), PhaseChanges, BeforeStop);
	TestFalse(TEXT("Stopped clock rejects ready"), Session->Ready());
	Session->OnChanged().Remove(Handle);

	TStrongObjectPtr<UFieldMatchSession> Defaults(NewObject<UFieldMatchSession>(World));
	Defaults->InitializeFixture();
	TestTrue(TEXT("Start default clock"), Defaults->ConfigureAndStartClock(World));
	TestEqual(TEXT("Default deployment is 45 seconds"), Defaults->GetSnapshot().SecondsRemaining, 45);
	const float DefaultStart = World->GetTimeSeconds();
	TestTrue(TEXT("Default ready"), Defaults->Ready());
	TestEqual(TEXT("Default spectating is 45 seconds"), Defaults->GetSnapshot().SecondsRemaining, 45);
	for (int32 Frame = 0; Frame < 600 && Defaults->GetSnapshot().Phase == EFieldMatchPhase::Spectating; ++Frame)
	{
		TestWorld.TickTestWorld(.1f);
	}
	TestTrue(TEXT("Default spectating ends"), Defaults->GetSnapshot().Phase == EFieldMatchPhase::Settlement);
	TestEqual(TEXT("Default settlement is 5 seconds"), Defaults->GetSnapshot().SecondsRemaining, 5);
	for (int32 Frame = 0; Frame < 100 && Defaults->GetSnapshot().Phase == EFieldMatchPhase::Settlement; ++Frame)
	{
		TestWorld.TickTestWorld(.1f);
	}
	TestTrue(TEXT("Default cycle returns to deployment"), Defaults->GetSnapshot().Phase == EFieldMatchPhase::Deployment);
	TestTrue(TEXT("Default cycle completes within 120 seconds including deployment"), World->GetTimeSeconds() - DefaultStart + 45.0f <= 120.0f);
	Defaults->StopClock();
	TestFalse(TEXT("Stopped deployment rejects ready"), Defaults->Ready());
	return true;
}
#endif
