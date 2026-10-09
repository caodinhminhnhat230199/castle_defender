#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Core/SandboxEnemyRespawner.h"
#include "Tests/EnemyTestFixture.h"
#include "Combat/CombatLibrary.h"
#include "Combat/HealthComponent.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"

namespace
{
	struct FRespawnerFixture : FEnemyTestWorld
	{
		ASandboxEnemyRespawner* Spawner = World->SpawnActor<ASandboxEnemyRespawner>();
		FRespawnerFixture(int32 Count = 1, bool bEnabled = true)
		{
			Spawner->EnemyDefinition = MakeTestEnemyDefinition(Spawner);
			Spawner->Count = Count;
			Spawner->bEnabled = bEnabled;
			Spawner->SpawnRadius = 0.f;
			Spawner->RespawnDelay = 0.5f; // Fixed fixture; saved preset remains five seconds.
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
			World->GetWorldSettings()->NotifyBeginPlay(); // No GameMode in the isolated fixture.
		}
		~FRespawnerFixture() { World->SetBegunPlay(false); } // Base fixture routes actor EndPlay before cleanup.
		void Advance(float Seconds)
		{
			TGuardValue<uint64> FrameGuard(GFrameCounter, GFrameCounter);
			for (int32 Step = 0; Step < FMath::RoundToInt(Seconds * 100.f); ++Step)
			{
				++GFrameCounter; // TimerManager only ticks once per engine frame.
				World->Tick(LEVELTICK_TimeOnly, 0.01f);
				World->GetTimerManager().Tick(0.01f); // TimeOnly deliberately excludes engine timers.
			}
		}
		void Kill(AEnemyCharacter* Enemy)
		{
			FCombatHit Hit;
			Hit.Damage = 100.f;
			UCombatLibrary::DeliverHit(Enemy, Hit);
		}
	};
}

BEGIN_DEFINE_SPEC(FSandboxRespawnerSpec, "CastleDefender.Combat.Sandbox.Respawner", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FSandboxRespawnerSpec)

void FSandboxRespawnerSpec::Define()
{
	It("initializes from the archetype before BeginPlay and replaces a killed enemy after the delay", [this]()
	{
		FRespawnerFixture F;
		TestEqual("Duel alive", F.Spawner->GetAliveCount(), 1);
		AEnemyCharacter* First = F.Spawner->GetTrackedEnemies()[0];
		TestEqual("Health from definition", First->GetHealthComponent()->GetCurrentHealth(), 100.f);
		F.Kill(First);
		TestEqual("Dead body is not alive capacity", F.Spawner->GetAliveCount(), 0);
		TestEqual("Pending replacement", F.Spawner->GetPendingCount(), 1);
		F.Advance(0.4f);
		TestEqual("No early respawn", F.Spawner->GetAliveCount(), 0);
		F.Advance(0.2f);
		TestEqual("Capacity restored", F.Spawner->GetAliveCount(), 1);
		if (TestEqual("One tracked replacement", F.Spawner->GetTrackedEnemies().Num(), 1))
		{
			TestNotEqual("Fresh enemy", F.Spawner->GetTrackedEnemies()[0], First);
		}
		TestEqual("No pending duplicate", F.Spawner->GetPendingCount(), 0);
	});

	It("keeps separate death deadlines and does not postpone the first enemy when the second dies", [this]()
	{
		FRespawnerFixture F(2);
		const auto Enemies = F.Spawner->GetTrackedEnemies();
		F.Kill(Enemies[0]);
		F.Advance(0.3f);
		F.Kill(Enemies[1]);
		F.Advance(0.3f);
		TestEqual("First deadline", F.Spawner->GetAliveCount(), 1);
		TestEqual("Second still waiting", F.Spawner->GetPendingCount(), 1);
		F.Advance(0.3f);
		TestEqual("Second deadline", F.Spawner->GetAliveCount(), 2);
	});

	It("changes capacity live, cancels removed slots and preserves unrelated enemies", [this]()
	{
		FRespawnerFixture F(3);
		AEnemyCharacter* Unrelated = F.World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(), FTransform(FVector(1000, 0, 100)));
		Unrelated->InitFromSpawn(F.Spawner->EnemyDefinition, FEnemySpawnParams());
		Unrelated->FinishSpawning(FTransform(FVector(1000, 0, 100)));
		F.Kill(F.Spawner->GetTrackedEnemies()[2]);
		F.Spawner->SetCount(1);
		TestEqual("Reduced live", F.Spawner->GetAliveCount(), 1);
		TestEqual("Canceled removed pending", F.Spawner->GetPendingCount(), 0);
		F.Advance(0.7f);
		TestEqual("No late extra spawn", F.Spawner->GetAliveCount(), 1);
		F.Spawner->SetCount(99);
		TestEqual("Clamp capacity", F.Spawner->Count, 3);
		TestEqual("Increased live", F.Spawner->GetAliveCount(), 3);
		F.Spawner->SetEnabled(false);
		TestEqual("Disabled preset", F.Spawner->GetAliveCount(), 0);
		TestFalse("Unrelated survives", Unrelated->IsActorBeingDestroyed());
		F.Spawner->SetEnabled(true);
		TestEqual("Enabled again", F.Spawner->GetAliveCount(), 3);
	});

	It("replaces explicit despawn and out-of-world removal once and cleans up timers on destruction", [this]()
	{
		FRespawnerFixture F(2);
		const auto Enemies = F.Spawner->GetTrackedEnemies();
		Enemies[0]->Despawn();
		Enemies[1]->Destroy(); // Enemy EndPlay owns the fallback OutOfWorld report.
		TestEqual("Two pending", F.Spawner->GetPendingCount(), 2);
		F.Advance(0.6f);
		TestEqual("Two replacements", F.Spawner->GetAliveCount(), 2);
		if (!TestEqual("Replacement present", F.Spawner->GetTrackedEnemies().Num(), 2)) { return; }
		F.Kill(F.Spawner->GetTrackedEnemies()[0]);
		F.Spawner->Destroy();
		F.Advance(0.7f);
		TestEqual("Destroyed owner has no timers", F.Spawner->GetPendingCount(), 0);
	});

	It("keeps respawn on world time while an individual enemy is dilated", [this]()
	{
		FRespawnerFixture F;
		AEnemyCharacter* Enemy = F.Spawner->GetTrackedEnemies()[0];
		Enemy->CustomTimeDilation = 0.01f;
		F.Kill(Enemy);
		F.Advance(0.6f);
		TestEqual("World-time replacement", F.Spawner->GetAliveCount(), 1);
	});

	It("supports a disabled Pair preset and zero-delay replacement on the next tick", [this]()
	{
		FRespawnerFixture F(2, false);
		TestEqual("No initial enemies", F.Spawner->GetAliveCount(), 0);
		F.Spawner->RespawnDelay = 0.f;
		F.Spawner->SetEnabled(true);
		F.Kill(F.Spawner->GetTrackedEnemies()[0]);
		TestEqual("No recursive spawn", F.Spawner->GetAliveCount(), 1);
		F.Advance(0.02f);
		TestEqual("Next tick capacity", F.Spawner->GetAliveCount(), 2);
	});
}
#endif
