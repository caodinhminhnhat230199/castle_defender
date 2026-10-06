#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyAttackFixture.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"

using namespace EnemyAttackTest;

namespace
{
	/** A hero hit on the enemy through the shared contract. */
	void HitEnemy(FAttackFixture& Fixture, float Damage, float PoiseDamage)
	{
		FCombatHit Hit;
		Hit.Instigator = Fixture.Hero;
		Hit.SourceLayer = ECombatLayer::Hero;
		Hit.Damage = Damage;
		Hit.PoiseDamage = PoiseDamage;
		UCombatLibrary::DeliverHit(Fixture.Enemy, Hit);
	}

	bool IsStaggered(const FAttackFixture& Fixture) { return Fixture.Enemy->GetCombatStateComponent()->HasState(GameTags::State_Combat_Staggered); }
}

BEGIN_DEFINE_SPEC(FEnemyStaggerSpec, "CastleDefender.Enemy.Stagger", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FEnemyStaggerSpec)

void FEnemyStaggerSpec::Define()
{
	LatentIt("poise break in the wind-up cancels the attack, holds all action, then acts again within one decision interval", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(1); // Heavy: the longest wind-up
		struct FProbe
		{
			bool bBroke = false; bool bLeftStaggered = false; bool bStaggerMontage = false; bool bStateChanged = false; bool bFeedbackChanged = false; bool bMoved = false;
			int32 TelegraphsAtBreak = 0; double StaggerEnded = -1.0; double ActedAgain = -1.0;
		};
		TSharedRef<FProbe> Probe = MakeShared<FProbe>();
		RunFrames(Fixture, Done, [Fixture, Probe]()
		{
			const EEnemyBrainState State = Fixture->Brain()->GetState();
			if (!Probe->bBroke && State == EEnemyBrainState::Attacking)
			{
				Probe->bBroke = true;
				HitEnemy(*Fixture, 1.f, Fixture->Enemy->GetCombatStateComponent()->GetMaxPoise());
				// Baseline after the break: it plays its own Staggered feedback synchronously.
				Probe->TelegraphsAtBreak = Fixture->Feedback->PlayedCount;
				Probe->bStaggerMontage = Fixture->Enemy->GetMesh()->GetAnimInstance()->Montage_IsPlaying(
					LoadObject<UAnimMontage>(nullptr, TEXT("/Game/CastleDefender/Enemy/AM_Enemy_Melee_Stagger.AM_Enemy_Melee_Stagger")));
				return true;
			}
			if (Probe->bBroke && Probe->StaggerEnded < 0.0)
			{
				if (IsStaggered(*Fixture))
				{
					// While Staggered: same state, no new attack, no movement.
					Probe->bStateChanged |= State != EEnemyBrainState::Staggered;
					Probe->bFeedbackChanged |= Fixture->Feedback->PlayedCount != Probe->TelegraphsAtBreak;
					Probe->bMoved |= Fixture->Enemy->GetVelocity().Size2D() > 1.f;
				}
				else
				{
					Probe->StaggerEnded = Fixture->Now();
				}
			}
			if (Probe->StaggerEnded >= 0.0 && Probe->ActedAgain < 0.0 && State != EEnemyBrainState::Staggered)
			{
				Probe->bLeftStaggered = true;
				Probe->ActedAgain = Fixture->Now();
			}
			return Probe->ActedAgain < 0.0;
		}, [this, Fixture, Probe]()
		{
			TestTrue("Poise broke during the wind-up", Probe->bBroke);
			TestTrue("Stagger presentation plays the stagger montage", Probe->bStaggerMontage);
			TestFalse("No damage from the cancelled attack", Fixture->HeroDamaged());
			TestFalse("State stays Staggered", Probe->bStateChanged);
			TestFalse("No new telegraph while Staggered", Probe->bFeedbackChanged);
			TestFalse("No movement", Probe->bMoved);
			TestTrue("Staggered ended", Probe->StaggerEnded >= 0.0);
			TestTrue(FString::Printf(TEXT("Acted again %.3f s after the state ended"), Probe->ActedAgain - Probe->StaggerEnded),
				Probe->bLeftStaggered && Probe->ActedAgain - Probe->StaggerEnded <= Fixture->Definition->DecisionInterval + FrameTime);
		});
	});

	LatentIt("poise break at hit-window start deals no damage from that attack", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(0);
		TSharedRef<bool> bBroke = MakeShared<bool>(false);
		// Inside the window-open event, before the first sweep of that frame.
		Fixture->Enemy->GetMeleeTraceComponent()->OnHitWindowBegin.AddLambda([Raw = &Fixture.Get(), bBroke]()
		{
			if (*bBroke) { return; }
			*bBroke = true;
			HitEnemy(*Raw, 1.f, Raw->Enemy->GetCombatStateComponent()->GetMaxPoise());
		});
		RunFrames(Fixture, Done, [Fixture, bBroke]()
		{
			// Stop when the stagger ends: a new attack may follow and is allowed to hit.
			return !*bBroke || (IsStaggered(*Fixture) && !Fixture->HeroDamaged());
		}, [this, Fixture, bBroke]()
		{
			TestTrue("Poise broke at hit-window start", *bBroke);
			TestFalse("Window closed by the stagger", Fixture->Enemy->GetMeleeTraceComponent()->IsHitWindowActive());
			TestFalse("No damage from that attack", Fixture->HeroDamaged());
		});
	});

	LatentIt("a hit without poise break never cancels the attack", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(0);
		TSharedRef<bool> bHit = MakeShared<bool>(false);
		RunFrames(Fixture, Done, [Fixture, bHit]()
		{
			if (!*bHit && Fixture->Brain()->GetState() == EEnemyBrainState::Attacking)
			{
				*bHit = true;
				HitEnemy(*Fixture, 5.f, Fixture->Enemy->GetCombatStateComponent()->GetMaxPoise() * 0.5f);
			}
			return !Fixture->HeroDamaged();
		}, [this, Fixture, bHit]()
		{
			TestTrue("Enemy was hit during the wind-up", *bHit);
			TestFalse("Not Staggered", IsStaggered(*Fixture));
			TestTrue("Its attack still landed", Fixture->HeroDamaged());
			TestEqual("Enemy took the damage", Fixture->Enemy->GetHealthComponent()->GetCurrentHealth(), Fixture->Definition->MaxHealth - 5.f);
		});
	});
}
#endif
