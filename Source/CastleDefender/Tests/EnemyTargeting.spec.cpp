#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Enemy/EnemyBrainComponent.h"
#include "Enemy/EnemyTargeting.h"
#include "Tests/EnemyTestFixture.h"
#include "Combat/CombatLibrary.h"
#include "Algo/Count.h"
#include "Containers/Ticker.h"
#include "Hero/HeroCharacter.h"
#include "TimerManager.h"

namespace
{
	FEnemyTargetCandidate Candidate(EEnemyTargetKind Kind, float Distance, bool bRecentAttacker = false)
	{
		FEnemyTargetCandidate Result;
		Result.Kind = Kind;
		Result.DistanceSq = Distance * Distance;
		Result.bRecentAttacker = bRecentAttacker;
		return Result;
	}

	/** Enemies at the origin with the shared fixture archetype; a hero placed outside the 600 cm aggro radius. */
	struct FBrainFixture : FEnemyTestWorld
	{
		UEnemyArchetypeDefinition* Definition = MakeTestEnemyDefinition(GetTransientPackage());
		TArray<AEnemyCharacter*> Enemies;
		AHeroCharacter* Hero = nullptr;

		AEnemyCharacter* Spawn()
		{
			AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
			Enemy->InitFromSpawn(Definition, FEnemySpawnParams());
			return Enemies.Add_GetRef(Enemy);
		}
		void BeginPlay()
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Hero = World->SpawnActor<AHeroCharacter>(FVector(1000.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
			World->InitializeActorsForPlay(FURL());
			for (AEnemyCharacter* Enemy : Enemies) { Enemy->DispatchBeginPlay(); }
		}
		UEnemyBrainComponent* Brain(int32 Index = 0) const { return Enemies[Index]->GetBrainComponent(); }
	};
}

BEGIN_DEFINE_SPEC(FEnemyTargetingSpec, "CastleDefender.Enemy.Targeting", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	TUniquePtr<FBrainFixture> Fixture;
END_DEFINE_SPEC(FEnemyTargetingSpec)

void FEnemyTargetingSpec::Define()
{
	using enum EEnemyTargetKind;
	const TArray<EEnemyTargetKind> HeroFirst = { Hero, Soldier };

	Describe("PickTarget", [this, HeroFirst]()
	{
		It("returns none without candidates or when no kind is in the priority list", [this, HeroFirst]()
		{
			TestEqual("Empty", EnemyTargeting::PickTarget({}, HeroFirst), INDEX_NONE);
			const TArray<FEnemyTargetCandidate> Towers = { Candidate(CombatTower, 100.f) };
			TestEqual("Unlisted kind", EnemyTargeting::PickTarget(Towers, HeroFirst), INDEX_NONE);
		});

		It("ranks by priority before attacker and distance", [this, HeroFirst]()
		{
			const TArray<FEnemyTargetCandidate> Candidates = { Candidate(Soldier, 100.f, true), Candidate(Hero, 500.f) };
			TestEqual("Hero listed first wins", EnemyTargeting::PickTarget(Candidates, HeroFirst), 1);
			TestEqual("Reordered list flips it", EnemyTargeting::PickTarget(Candidates, { Soldier, Hero }), 0);
		});

		It("breaks priority ties by recent attacker, then by distance", [this, HeroFirst]()
		{
			const TArray<FEnemyTargetCandidate> Attacker = { Candidate(Soldier, 100.f), Candidate(Soldier, 400.f, true) };
			TestEqual("Recent attacker beats nearer", EnemyTargeting::PickTarget(Attacker, HeroFirst), 1);
			const TArray<FEnemyTargetCandidate> Distance = { Candidate(Soldier, 400.f), Candidate(Soldier, 100.f), Candidate(Soldier, 250.f) };
			TestEqual("Nearest", EnemyTargeting::PickTarget(Distance, HeroFirst), 1);
		});
	});

	Describe("PickAttack", [this]()
	{
		const auto Attack = [](float Range, float Weight)
		{
			FEnemyAttackDefinition Result;
			Result.Range = Range;
			Result.Weight = Weight;
			return Result;
		};

		It("returns none when no attack reaches the gap", [this, Attack]()
		{
			const TArray<FEnemyAttackDefinition> Attacks = { Attack(150.f, 1.f), Attack(200.f, 1.f) };
			TestEqual("Out of range", EnemyTargeting::PickAttack(Attacks, {}, 250.f, 0.0, 0.5f), INDEX_NONE);
			TestEqual("Reaches only the long one", EnemyTargeting::PickAttack(Attacks, {}, 180.f, 0.0, 0.f), 1);
		});

		It("skips attacks still on cooldown", [this, Attack]()
		{
			const TArray<FEnemyAttackDefinition> Attacks = { Attack(150.f, 1.f), Attack(150.f, 1.f) };
			const TArray<double> Ready = { 10.0, 0.0 };
			TestEqual("First cooling down", EnemyTargeting::PickAttack(Attacks, Ready, 100.f, 5.0, 0.f), 1);
			TestEqual("Ready again at its time", EnemyTargeting::PickAttack(Attacks, Ready, 100.f, 10.0, 0.f), 0);
			TestEqual("All cooling down", EnemyTargeting::PickAttack(Attacks, { 10.0, 10.0 }, 100.f, 5.0, 0.f), INDEX_NONE);
		});

		It("picks by weight with the roll", [this, Attack]()
		{
			const TArray<FEnemyAttackDefinition> Attacks = { Attack(150.f, 2.f), Attack(150.f, 1.f) };
			TestEqual("Roll 0", EnemyTargeting::PickAttack(Attacks, {}, 100.f, 0.0, 0.f), 0);
			TestEqual("Roll inside the first 2/3", EnemyTargeting::PickAttack(Attacks, {}, 100.f, 0.0, 0.6f), 0);
			TestEqual("Roll past 2/3", EnemyTargeting::PickAttack(Attacks, {}, 100.f, 0.0, 0.7f), 1);
			TestEqual("Roll 1", EnemyTargeting::PickAttack(Attacks, {}, 100.f, 0.0, 1.f), 1);
		});
	});

	Describe("Brain", [this]()
	{
		BeforeEach([this]() { Fixture = MakeUnique<FBrainFixture>(); });
		AfterEach([this]() { Fixture.Reset(); });

		It("never ticks and starts Idle", [this]()
		{
			Fixture->Spawn();
			Fixture->BeginPlay();
			TestFalse("Tick disabled", Fixture->Brain()->PrimaryComponentTick.bCanEverTick);
			TestEqual("Idle", Fixture->Brain()->GetState(), EEnemyBrainState::Idle);
		});

		It("stays Idle with the hero outside the aggro radius and engages once it enters", [this]()
		{
			Fixture->Spawn();
			Fixture->BeginPlay();
			UEnemyBrainComponent* Brain = Fixture->Brain();
			Brain->Decide();
			TestEqual("Idle outside", Brain->GetState(), EEnemyBrainState::Idle);
			Fixture->Hero->SetActorLocation(FVector(300.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
			Brain->Decide();
			TestEqual("Engage inside", Brain->GetState(), EEnemyBrainState::Engage);
			TestEqual("Targets the hero", Brain->GetTarget(), static_cast<AActor*>(Fixture->Hero));
			Fixture->Hero->Destroy();
			Brain->Decide();
			TestEqual("Idle when the target is gone", Brain->GetState(), EEnemyBrainState::Idle);
			TestNull("Target cleared", Brain->GetTarget());
		});

		It("holds Paused until resumed and ends Dead on death", [this]()
		{
			Fixture->Spawn();
			Fixture->BeginPlay();
			UEnemyBrainComponent* Brain = Fixture->Brain();
			Fixture->Hero->SetActorLocation(FVector(300.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
			Brain->PauseDecisions();
			Brain->Decide();
			TestEqual("Paused ignores decisions", Brain->GetState(), EEnemyBrainState::Paused);
			Brain->ResumeDecisions();
			Brain->Decide();
			TestEqual("Re-evaluates after resume", Brain->GetState(), EEnemyBrainState::Engage);
			FCombatHit Hit;
			Hit.Damage = Fixture->Definition->MaxHealth;
			UCombatLibrary::DeliverHit(Fixture->Enemies[0], Hit);
			TestEqual("Dead", Brain->GetState(), EEnemyBrainState::Dead);
			Brain->Decide();
			TestEqual("Dead is terminal", Brain->GetState(), EEnemyBrainState::Dead);
		});

		LatentIt("decides at the data interval and spreads first decisions of enemies spawned together", [this](const FDoneDelegate& Done)
		{
			Fixture->Definition->DecisionInterval = 0.25f;
			for (int32 Index = 0; Index < 20; ++Index) { Fixture->Spawn(); }
			Fixture->BeginPlay();
			TSharedRef<int32> DecidedEarly = MakeShared<int32>(-1);
			// No engine context ticks this world: advance game time and its timers once per real frame
			// (the timer manager ticks at most once per frame). Timers set before the first tick activate at its end, so the
			// 5 s window and the half-interval spread check both start at 0.05 s.
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Done, DecidedEarly](float)
			{
				UWorld* World = Fixture->World;
				World->TimeSeconds += 0.05;
				World->GetTimerManager().Tick(0.05f);
				if (*DecidedEarly < 0 && World->TimeSeconds >= 0.175 - KINDA_SMALL_NUMBER)
				{
					*DecidedEarly = Algo::CountIf(Fixture->Enemies, [](const AEnemyCharacter* E) { return E->GetBrainComponent()->GetDecisionCount() > 0; });
				}
				if (World->TimeSeconds < 5.05 - KINDA_SMALL_NUMBER) { return true; }
				// First fire at activation + offset in [0, 0.25), then every 0.25 s: 20 or 21 decisions in 5 s.
				for (const AEnemyCharacter* Enemy : Fixture->Enemies)
				{
					const int32 Count = Enemy->GetBrainComponent()->GetDecisionCount();
					TestTrue(FString::Printf(TEXT("%s cadence (%d decisions)"), *Enemy->GetName(), Count), Count >= 20 && Count <= 21);
				}
				TestTrue(FString::Printf(TEXT("Spread first decisions (%d of 20 by half an interval)"), *DecidedEarly), *DecidedEarly > 0 && *DecidedEarly < 20);
				Done.Execute();
				return false;
			}));
		});
	});
}
#endif
