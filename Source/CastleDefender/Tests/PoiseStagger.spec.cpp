#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Tests/EnemyTestFixture.h"
#include "Tests/FeedbackTestListener.h"
#include "Tests/CombatTestListener.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/Ticker.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Enemy/EnemyBrainComponent.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/EnemyArchetypeDefinition.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/HeroCombatComponent.h"

namespace
{
	constexpr float FrameTime = 1.f / 60.f;

	struct FPoiseStaggerFixture : FEnemyTestWorld
	{
		UEnemyArchetypeDefinition* Definition = nullptr;
		AEnemyCharacter* Enemy = nullptr;
		AHeroCharacter* Hero = nullptr;
		UFeedbackTestListener* Feedback = nullptr;
		float HeroStartHealth = 0.f;

		explicit FPoiseStaggerFixture(float MaxPoise = 50.f, int32 AttackIndex = 0)
		{
			AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.f, 0.f, -50.f), FRotator::ZeroRotator);
			Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			Floor->SetActorScale3D(FVector(20.f, 20.f, 1.f));

			Definition = DuplicateObject(LoadObject<UEnemyArchetypeDefinition>(nullptr, TEXT("/Game/CastleDefender/Enemy/DA_Enemy_Test.DA_Enemy_Test")), GetTransientPackage());
			Definition->CombatState.MaxPoise = MaxPoise;
			if (Definition->Attacks.IsValidIndex(AttackIndex))
			{
				Definition->Attacks = { Definition->Attacks[AttackIndex] };
			}

			const FTransform EnemyAt(FVector(0.f, 0.f, 100.f));
			Enemy = World->SpawnActorDeferred<AEnemyCharacter>(LoadClass<AEnemyCharacter>(nullptr, TEXT("/Game/CastleDefender/Enemy/BP_Enemy_Base.BP_Enemy_Base_C")),
				EnemyAt, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Enemy->InitFromSpawn(Definition, FEnemySpawnParams());
			Enemy->FinishSpawning(EnemyAt);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Hero = World->SpawnActor<AHeroCharacter>(FVector(150.f, 0.f, 100.f), FRotator(0.f, 180.f, 0.f), Params);
			Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(
				LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
			Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
			Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());

			World->InitializeActorsForPlay(FURL());
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (!It->HasActorBegunPlay()) { It->DispatchBeginPlay(); }
			}
			HeroStartHealth = Hero->GetHealthComponent()->GetCurrentHealth();

			Feedback = NewObject<UFeedbackTestListener>();
			if (UFeedbackSubsystem* FeedbackSys = World->GetSubsystem<UFeedbackSubsystem>())
			{
				FeedbackSys->OnFeedbackPlayed.AddDynamic(Feedback, &UFeedbackTestListener::HandlePlayed);
			}
		}

		void Tick() { World->Tick(LEVELTICK_All, FrameTime); }
		double Now() const { return World->GetTimeSeconds(); }
		bool HeroDamaged() const { return Hero->GetHealthComponent()->GetCurrentHealth() < HeroStartHealth; }
		bool IsStaggered() const { return Enemy->GetCombatStateComponent()->HasState(GameTags::State_Combat_Staggered); }
	};

	inline void RunFrames(TSharedRef<FPoiseStaggerFixture> Fixture, const FDoneDelegate& Done,
		TFunction<bool()> Step, TFunction<void()> Finish, double MaxDuration = 4.5)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Fixture, Done, Step, Finish, MaxDuration](float)
		{
			Fixture->Tick();
			if (Step() && Fixture->Now() < MaxDuration) { return true; }
			Finish();
			Done.Execute();
			return false;
		}));
	}
	void ResetHeroToIdle(AHeroCharacter* Hero)
	{
		if (Hero && Hero->GetMesh() && Hero->GetMesh()->GetAnimInstance())
		{
			Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
			Hero->GetMesh()->TickAnimation(0.01f, false);
			Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
		}
	}
}

BEGIN_DEFINE_SPEC(FPoiseStaggerSpec, "CastleDefender.Combat.States.PoiseBreak", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FPoiseStaggerSpec)

void FPoiseStaggerSpec::Define()
{
	LatentIt("hero Heavy + Light + Light staggers P0 melee enemy, cancels attack montage and plays Staggered feedback once (AC-SYN-06)", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FPoiseStaggerFixture> Fixture = MakeShared<FPoiseStaggerFixture>(50.f, 1); // Heavy enemy attack (longest wind-up)
		struct FState
		{
			bool bHeavyHit = false;
			bool bLight1Hit = false;
			bool bBrokePoise = false;
			bool bAttackWasActive = false;
			bool bMontageCancelled = false;
			double BreakTime = -1.0;
		};
		TSharedRef<FState> S = MakeShared<FState>();

		RunFrames(Fixture, Done, [Fixture, S]()
		{
			const EEnemyBrainState BrainState = Fixture->Enemy->GetBrainComponent()->GetState();
			if (!S->bBrokePoise)
			{
				// Wait until enemy begins attacking so we can verify the attack is cancelled by stagger
				if (BrainState == EEnemyBrainState::Attacking)
				{
					S->bAttackWasActive = true;

					// 1. Heavy attack: 40 poise damage
					Fixture->Hero->GetCombatComponent()->RequestAction(EHeroAction::Heavy);
					Fixture->Hero->GetMeleeTraceComponent()->BeginHitWindow();
					Fixture->Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture->Enemy);
					Fixture->Hero->GetMeleeTraceComponent()->EndHitWindow();
					S->bHeavyHit = true;
					ResetHeroToIdle(Fixture->Hero);

					// 2. Light attack 1: 5 poise damage
					Fixture->Hero->GetCombatComponent()->RequestAction(EHeroAction::Light);
					Fixture->Hero->GetMeleeTraceComponent()->BeginHitWindow();
					Fixture->Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture->Enemy);
					Fixture->Hero->GetMeleeTraceComponent()->EndHitWindow();
					S->bLight1Hit = true;
					ResetHeroToIdle(Fixture->Hero);

					// 3. Light attack 2: 5 poise damage -> breaks 50 MaxPoise
					Fixture->Hero->GetCombatComponent()->RequestAction(EHeroAction::Light);
					Fixture->Hero->GetMeleeTraceComponent()->BeginHitWindow();
					Fixture->Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture->Enemy);
					Fixture->Hero->GetMeleeTraceComponent()->EndHitWindow();

					S->bBrokePoise = Fixture->IsStaggered();
					S->BreakTime = Fixture->Now();

					UAnimMontage* EnemyMontage = Fixture->Enemy->GetRuntimeParams().Attacks[0].Montage;
					S->bMontageCancelled = !Fixture->Enemy->GetMesh()->GetAnimInstance()->Montage_IsPlaying(EnemyMontage);
					return true;
				}
				return true;
			}

			// While Staggered, continue ticking until stagger duration ends
			if (Fixture->IsStaggered())
			{
				return true;
			}

			// Stagger finished
			return false;
		}, [this, Fixture, S]()
		{
			TestTrue("Enemy was attacking when poise broke", S->bAttackWasActive);
			TestTrue("Heavy attack landed", S->bHeavyHit);
			TestTrue("Light attack landed", S->bLight1Hit);
			TestTrue("Poise broke into Staggered", S->bBrokePoise);
			TestTrue("Attack montage was cancelled", S->bMontageCancelled);
			TestFalse("Hero took no damage from cancelled attack", Fixture->HeroDamaged());
			const int32 StaggerFeedback = Fixture->Feedback->PlayedTags.FilterByPredicate([](const FGameplayTag& Tag)
			{
				return Tag == FeedbackTags::State_Staggered_Applied;
			}).Num();
			TestEqual("Feedback.State.Staggered.Applied played exactly once", StaggerFeedback, 1);
		});
	});

	It("enemy with MaxPoise 0 does not enter Staggered from poise damage (regression guard)", [this]()
	{
		FPoiseStaggerFixture Fixture(50.f, 0);
		FCombatStateConfig ZeroPoiseConfig;
		ZeroPoiseConfig.MaxPoise = 0.f;
		ZeroPoiseConfig.PoiseRegenDelay = 2.f;
		ZeroPoiseConfig.PoiseRegenRate = 25.f;
		ZeroPoiseConfig.StaggerDuration = 1.5f;
		Fixture.Enemy->GetCombatStateComponent()->Init(ZeroPoiseConfig);
		TestEqual("MaxPoise is 0", Fixture.Enemy->GetCombatStateComponent()->GetMaxPoise(), 0.f);

		// Heavy + Light + Light
		Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Heavy);
		Fixture.Hero->GetMeleeTraceComponent()->BeginHitWindow();
		Fixture.Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture.Enemy);
		Fixture.Hero->GetMeleeTraceComponent()->EndHitWindow();
		ResetHeroToIdle(Fixture.Hero);

		Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Light);
		Fixture.Hero->GetMeleeTraceComponent()->BeginHitWindow();
		Fixture.Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture.Enemy);
		Fixture.Hero->GetMeleeTraceComponent()->EndHitWindow();
		ResetHeroToIdle(Fixture.Hero);

		Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Light);
		Fixture.Hero->GetMeleeTraceComponent()->BeginHitWindow();
		Fixture.Hero->GetMeleeTraceComponent()->TryHitTarget(Fixture.Enemy);
		Fixture.Hero->GetMeleeTraceComponent()->EndHitWindow();

		TestFalse("Enemy with MaxPoise 0 is NOT staggered", Fixture.IsStaggered());
		TestEqual("Poise remains 0", Fixture.Enemy->GetCombatStateComponent()->GetCurrentPoise(), 0.f);
	});

	LatentIt("hero parry during enemy hit window applies 60 poise damage and staggers enemy", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FPoiseStaggerFixture> Fixture = MakeShared<FPoiseStaggerFixture>(50.f, 0);
		UCombatTestListener* Listener = NewObject<UCombatTestListener>();
		Fixture->Hero->GetCombatComponent()->OnParrySucceeded.AddDynamic(Listener, &UCombatTestListener::HandleParrySucceeded);

		TSharedRef<bool> bParryWindowOpened = MakeShared<bool>(false);

		// When enemy hit window begins: hero requests Parry and opens parry window
		Fixture->Enemy->GetMeleeTraceComponent()->OnHitWindowBegin.AddLambda([Raw = &Fixture.Get(), bParryWindowOpened]()
		{
			if (*bParryWindowOpened) { return; }
			*bParryWindowOpened = true;
			Raw->Hero->GetCombatComponent()->RequestAction(EHeroAction::Parry);
			Raw->Hero->GetCombatComponent()->OpenParryWindow();
		});

		RunFrames(Fixture, Done, [Fixture, Listener]()
		{
			if (Listener->ParrySucceededCount > 0 || Fixture->IsStaggered())
			{
				return false; // Parried and staggered!
			}
			return true;
		}, [this, Fixture, Listener, bParryWindowOpened]()
		{
			TestTrue("Parry window opened on enemy hit window begin", *bParryWindowOpened);
			TestTrue("Parry succeeded event fired", Listener->ParrySucceededCount > 0);
			TestTrue("Enemy is Staggered from parry poise damage", Fixture->IsStaggered());
			TestFalse("Hero took no damage", Fixture->HeroDamaged());
			const int32 ParryCount = Fixture->Feedback->PlayedTags.FilterByPredicate([](const FGameplayTag& Tag)
			{
				return Tag == FeedbackTags::Combat_Parry;
			}).Num();
			TestTrue("Feedback.Combat.Parry played", ParryCount >= 1);
			const int32 StaggerCount = Fixture->Feedback->PlayedTags.FilterByPredicate([](const FGameplayTag& Tag)
			{
				return Tag == FeedbackTags::State_Staggered_Applied;
			}).Num();
			TestTrue("Feedback.State.Staggered.Applied played", StaggerCount >= 1);
		});
	});

	LatentIt("poise regenerates after delay: 40 damage at t=0 leaves 10, after delay 2s + 1s regen reaches 35 (+-1)", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FPoiseStaggerFixture> Fixture = MakeShared<FPoiseStaggerFixture>(50.f, 0);
		struct FRegenState
		{
			bool bDamaged = false;
			double StartTime = 0.0;
			float PoiseAt1s = 0.f;
			float PoiseAt3s = 0.f;
			bool bChecked1s = false;
		};
		TSharedRef<FRegenState> S = MakeShared<FRegenState>();

		RunFrames(Fixture, Done, [Fixture, S]()
		{
			if (!S->bDamaged)
			{
				S->bDamaged = true;
				S->StartTime = Fixture->Now();
				Fixture->Enemy->GetCombatStateComponent()->ApplyPoiseDamage(40.f, Fixture->Hero);
				return true;
			}

			const double Elapsed = Fixture->Now() - S->StartTime;
			if (!S->bChecked1s && Elapsed >= 1.0)
			{
				S->bChecked1s = true;
				S->PoiseAt1s = Fixture->Enemy->GetCombatStateComponent()->GetCurrentPoise();
			}

			if (Elapsed >= 3.0)
			{
				S->PoiseAt3s = Fixture->Enemy->GetCombatStateComponent()->GetCurrentPoise();
				return false; // Done
			}

			return true;
		}, [this, Fixture, S]()
		{
			TestTrue("Poise damage applied", S->bDamaged);
			TestEqual("Poise at 1s is still 10 (inside delay 2.0s)", S->PoiseAt1s, 10.f, 0.5f);
			TestEqual("Poise at 3s regenerated to 35 (+-1)", S->PoiseAt3s, 35.f, 1.0f);
		});
	});
}

#endif
