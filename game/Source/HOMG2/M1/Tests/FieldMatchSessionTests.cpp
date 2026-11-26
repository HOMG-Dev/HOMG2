#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "M1/FieldGameMode.h"
#include "Tests/AutomationCommon.h"
#include "M1/FieldMatchSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFieldMatchSessionTest, "HOMG2.M1.MatchSession",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFieldMatchSessionTest::RunTest(const FString&)
{
	FTestWorldWrapper WorldA, WorldB;
	auto StartMatch = [this](FTestWorldWrapper& World) -> UFieldMatchSession*
	{
		if (!TestTrue(TEXT("Create standalone field world"), World.CreateTestWorld(EWorldType::Game)))
		{
			return nullptr;
		}
		UWorld* MatchWorld = World.GetTestWorld();
		MatchWorld->GetWorldSettings()->DefaultGameMode = AFieldGameMode::StaticClass();
		if (!TestTrue(TEXT("Begin play in field world"), World.BeginPlayInTestWorld()))
		{
			return nullptr;
		}
		AFieldGameMode* GameMode = MatchWorld->GetAuthGameMode<AFieldGameMode>();
		return GameMode ? GameMode->GetMatchSession() : nullptr;
	};
	UFieldMatchSession* A = StartMatch(WorldA);
	UFieldMatchSession* B = StartMatch(WorldB);
	if (!TestNotNull(TEXT("First GameMode owns a session"), A) ||
		!TestNotNull(TEXT("Second GameMode owns a session"), B))
	{
		return false;
	}
	const FFieldMatchSnapshot Initial = A->GetSnapshot();
	const FFieldMatchSnapshot Other = B->GetSnapshot();
	if (!TestEqual(TEXT("Fixture has one battalion"), Initial.Battalions.Num(), 1) ||
		!TestEqual(TEXT("Fixture has equipment"), Initial.Inventory.Num(), 1))
	{
		return false;
	}
	TestTrue(TEXT("Session IDs are distinct"), Initial.SessionId.IsValid() && Initial.SessionId != Other.SessionId);
	TestTrue(TEXT("Battalion has an ID"), Initial.Battalions[0].Id.IsValid());
	TestTrue(TEXT("Members have stable IDs"), Initial.Battalions[0].Members.Num() > 0 && Initial.Battalions[0].Members[0].Id.IsValid());
	TestTrue(TEXT("Equipment has an ID"), Initial.Inventory[0].Id.IsValid());
	int32 Notifications = 0;
	const FDelegateHandle Handle = A->OnChanged().AddLambda([&] { ++Notifications; });
	TestTrue(TEXT("Change gold"), A->SetGold(42));
	TestTrue(TEXT("Remove equipment"), A->RemoveEquipment(Initial.Inventory[0].Id));
	TestFalse(TEXT("No duplicate removal"), A->RemoveEquipment(Initial.Inventory[0].Id));
	TestEqual(TEXT("One notification per change"), Notifications, 2);
	TestEqual(TEXT("Late reader gets current gold"), A->GetSnapshot().Gold, 42);
	TestEqual(TEXT("Late reader gets current inventory"), A->GetSnapshot().Inventory.Num(), 0);
	TestEqual(TEXT("Other session balance isolated"), B->GetSnapshot().Gold, Other.Gold);
	TestEqual(TEXT("Other session inventory isolated"), B->GetSnapshot().Inventory.Num(), Other.Inventory.Num());
	TestEqual(TEXT("Earlier copy is unchanged"), Initial.Gold, 100);
	TestTrue(TEXT("Battalion ID stable after changes"), A->GetSnapshot().Battalions[0].Id == Initial.Battalions[0].Id);
	A->OnChanged().Remove(Handle);
	if (!TestTrue(TEXT("Exit the first match"), WorldA.DestroyTestWorld(false)))
	{
		return false;
	}
	UFieldMatchSession* Reopened = StartMatch(WorldA);
	if (!TestNotNull(TEXT("Reopened GameMode owns a session"), Reopened))
	{
		return false;
	}
	TestEqual(TEXT("Reopen restores initial gold"), Reopened->GetSnapshot().Gold, Initial.Gold);
	TestEqual(TEXT("Reopen restores initial inventory"), Reopened->GetSnapshot().Inventory.Num(), Initial.Inventory.Num());
	TestEqual(TEXT("Reopen starts in round one"), Reopened->GetSnapshot().Round, 1);
	TestEqual(TEXT("Reopen starts in deployment"), Reopened->GetSnapshot().Phase, EFieldMatchPhase::Deployment);
	TestNotEqual(TEXT("Reopen creates a fresh session ID"), Reopened->GetSnapshot().SessionId, Initial.SessionId);
	return true;
}
#endif
