#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Editor.h"
#include "Engine/World.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationEditorCommon.h"
#include "Framework/Dev/FrameworkDemo.h"
#include "Framework/Network/UnitFeed.h"
#include "Framework/Presentation/UnitPresenter.h"

namespace
{
class FCheckUnitReplication : public IAutomationLatentCommand
{
public:
    explicit FCheckUnitReplication(FAutomationTestBase& InTest) : Test(InTest) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() > Deadline)
        {
            Test.AddError(FString::Printf(TEXT("Timed out waiting for private replication at stage %d"), Stage));
            return true;
        }
        AFrameworkDemoController* Host = nullptr;
        AFrameworkDemoController* Remote = nullptr;
        for (const auto& Ctx : GEngine->GetWorldContexts())
        {
            auto* W = Ctx.World();
            if (!W || Ctx.WorldType != EWorldType::PIE) continue;
            for (auto It = W->GetPlayerControllerIterator(); It; ++It)
            {
                auto* PC = Cast<AFrameworkDemoController>(It->Get());
                if (!PC || !PC->IsLocalController() || !PC->GetUnitPresenter()) continue;
                if (W->GetNetMode() == NM_ListenServer) Host = PC;
                if (W->GetNetMode() == NM_Client) Remote = PC;
            }
        }
        if (!Host || !Remote) return false;
        const auto HostState = Host->GetUnitPresenter()->GetSelected();
        const auto RemoteState = Remote->GetUnitPresenter()->GetSelected();
        if (Stage == 0)
        {
            if (!HostState.Id.IsValid() || !RemoteState.Id.IsValid()) return false;
            Test.TestTrue(TEXT("Different players own different units"), HostState.Id != RemoteState.Id);
            Test.TestEqual(TEXT("One authorized remote snapshot"), Remote->GetUnitFeed()->GetVisible().Num(), 1);
            Test.TestFalse(TEXT("Host unit hidden on client"), Remote->GetUnitPresenter()->GetUnit(HostState.Id).Id.IsValid());
            Host->DemoDamage();
            Test.TestEqual(TEXT("Host presenter refreshed without OnRep roundtrip"), Host->GetUnitPresenter()->GetSelected().Condition, EUnitCondition::Critical);
            Test.TestEqual(TEXT("Host mutation isolated"), RemoteState.Condition, EUnitCondition::Healthy);
            Remote->DemoDamage(); // Real owner RPC to the server.
            ++Stage;
        }
        else if (Stage == 1)
        {
            if (RemoteState.Condition != EUnitCondition::Critical) return false;
            for (auto It = Host->GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                auto* PC = Cast<AFrameworkDemoController>(It->Get());
                if (PC && !PC->IsLocalController())
                {
                    Test.TestEqual(TEXT("Server snapshot matches remote"), PC->GetUnitFeed()->GetVisible()[0].Id, RemoteState.Id);
                    PC->GetUnitFeed()->Revoke(RemoteState.Id);
                    ++Stage;
                    break;
                }
            }
        }
        else if (Remote->GetUnitFeed()->GetVisible().IsEmpty())
        {
            Test.TestFalse(TEXT("Remote selection cleared"), RemoteState.Id.IsValid());
            Test.TestTrue(TEXT("Host selection survives"), HostState.Id.IsValid());
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase& Test;
    double Deadline = FPlatformTime::Seconds() + 90.;
    int32 Stage = 0;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnitNetworkTest, "HOMG2.Framework.PrivateReplication",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnitNetworkTest::RunTest(const FString&)
{
    if (!GEditor || GEditor->PlayWorld) { AddError(TEXT("Start this test outside PIE")); return false; }
    FRequestPlaySessionParams Params;
    Params.EditorPlaySettings = NewObject<ULevelEditorPlaySettings>();
    Params.EditorPlaySettings->SetPlayNetMode(PIE_ListenServer);
    Params.EditorPlaySettings->SetRunUnderOneProcess(true);
    Params.EditorPlaySettings->SetPlayNumberOfClients(2);
    Params.GameModeOverride = AFrameworkDemoMode::StaticClass();
    Params.bAllowOnlineSubsystem = false;
    GEditor->RequestPlaySession(Params);
    ADD_LATENT_AUTOMATION_COMMAND(FCheckUnitReplication(*this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
    return true;
}
#endif
