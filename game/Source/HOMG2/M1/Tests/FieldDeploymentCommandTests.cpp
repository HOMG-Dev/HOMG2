#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldDeploymentCommandTest, "HOMG2.M1.DeploymentCommand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldDeploymentCommandTest::RunTest(const FString&)
{
	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("Create world"), TestWorld.CreateTestWorld(EWorldType::Game))) return false;
	UWorld* World = TestWorld.GetTestWorld();
	auto* Session = NewObject<UFieldMatchSession>(World);
	Session->InitializeFixture();
	FFieldCommandResult Result = Session->SpendTestGold(10);
	TestTrue(TEXT("No active match rejected"), Result.Code == EFieldCommandCode::NoActiveMatch);
	TestFalse(TEXT("Inactive rejection has reason"), Result.Reason.IsEmpty());
	if (!TestTrue(TEXT("Begin play"), TestWorld.BeginPlayInTestWorld())) return false;
	if (!TestTrue(TEXT("Start clock"), Session->ConfigureAndStartClock(World))) return false;
	const FFieldMatchSnapshot Initial = Session->GetSnapshot();
	int32 Changes = 0;
	auto Handle = Session->OnChanged().AddLambda([&] { ++Changes; });
	Result = Session->SpendTestGold(10);
	TestTrue(TEXT("Deployment command succeeds"), Result.Succeeded());
	TestEqual(TEXT("Gold spent once"), Session->GetSnapshot().Gold, 90);
	TestEqual(TEXT("One change notification"), Changes, 1);
	Result = Session->SpendTestGold(0);
	TestTrue(TEXT("Invalid amount rejected"), Result.Code == EFieldCommandCode::InvalidAmount);
	TestFalse(TEXT("Invalid amount has reason"), Result.Reason.IsEmpty());
	Result = Session->SpendTestGold(1000);
	TestTrue(TEXT("Insufficient gold rejected"), Result.Code == EFieldCommandCode::InsufficientGold);
	TestFalse(TEXT("Rejection has reason"), Result.Reason.IsEmpty());
	TestEqual(TEXT("Failures leave balance"), Session->GetSnapshot().Gold, 90);
	TestEqual(TEXT("Failures do not notify"), Changes, 1);
	TestTrue(TEXT("Ready"), Session->Ready());
	const int32 Before = Changes;
	Result = Session->SpendTestGold(10);
	TestTrue(TEXT("Direct call during spectating rejected"), Result.Code == EFieldCommandCode::WrongPhase);
	TestFalse(TEXT("Phase rejection has reason"), Result.Reason.IsEmpty());
	TestEqual(TEXT("Rejection leaves balance"), Session->GetSnapshot().Gold, 90);
	TestEqual(TEXT("Rejections leave inventory count"), Session->GetSnapshot().Inventory.Num(), Initial.Inventory.Num());
	if (!Initial.Inventory.IsEmpty() && !Session->GetSnapshot().Inventory.IsEmpty())
	{
		TestTrue(TEXT("Rejections preserve inventory item"), Session->GetSnapshot().Inventory[0].Id == Initial.Inventory[0].Id);
	}
	TestEqual(TEXT("Rejection has no notification"), Changes, Before);
	Session->OnChanged().Remove(Handle);
	Session->StopClock();
	return true;
}
#endif
