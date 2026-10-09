#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/EnemyCombatTestLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

BEGIN_DEFINE_SPEC(FEnemyCombatSuiteSpec, "CastleDefender.Enemy.CombatSuite", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UWorld* World = nullptr;
END_DEFINE_SPEC(FEnemyCombatSuiteSpec)

void FEnemyCombatSuiteSpec::Define()
{
	BeforeEach([this]()
	{
		World = UWorld::CreateWorld(EWorldType::Game, false);
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
	});

	AfterEach([this]()
	{
		if (World)
		{
			World->BeginTearingDown();
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (It->HasActorBegunPlay())
				{
					It->RouteEndPlay(EEndPlayReason::Quit);
				}
			}
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			World = nullptr;
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
	});

	Describe("T-ENM-12 P0 Functional Scenarios", [this]()
	{
		It("FT_Enemy_AggroChase: idle outside aggro radius, engages inside aggro radius", [this]()
		{
			FString Message;
			const bool bPassed = UEnemyCombatTestLibrary::RunEnemyCombatScenario(World, TEXT("FT_Enemy_AggroChase"), Message);
			TestTrue(Message, bPassed);
		});

		It("FT_Enemy_TelegraphGap: wind-up exceeds min telegraph time before active hit window", [this]()
		{
			FString Message;
			const bool bPassed = UEnemyCombatTestLibrary::RunEnemyCombatScenario(World, TEXT("FT_Enemy_TelegraphGap"), Message);
			TestTrue(Message, bPassed);
		});

		It("FT_Enemy_StaggerCancel: poise break during wind-up cancels attack and staggers", [this]()
		{
			FString Message;
			const bool bPassed = UEnemyCombatTestLibrary::RunEnemyCombatScenario(World, TEXT("FT_Enemy_StaggerCancel"), Message);
			TestTrue(Message, bPassed);
		});

		It("FT_Enemy_DeathReportOnce: lethal hit reports removal once and clears pawn collision", [this]()
		{
			FString Message;
			const bool bPassed = UEnemyCombatTestLibrary::RunEnemyCombatScenario(World, TEXT("FT_Enemy_DeathReportOnce"), Message);
			TestTrue(Message, bPassed);
		});

		It("FT_Enemy_ParryStaggers: enemy hit during hero parry window is intercepted and staggers enemy", [this]()
		{
			FString Message;
			const bool bPassed = UEnemyCombatTestLibrary::RunEnemyCombatScenario(World, TEXT("FT_Enemy_ParryStaggers"), Message);
			TestTrue(Message, bPassed);
		});
	});
}

#endif
