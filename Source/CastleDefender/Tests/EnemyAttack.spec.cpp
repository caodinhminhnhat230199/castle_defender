#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyAttackFixture.h"

using namespace EnemyAttackTest;

BEGIN_DEFINE_SPEC(FEnemyAttackSpec, "CastleDefender.Enemy.Attack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	void TelegraphGap(int32 AttackIndex, FGameplayTag ExpectedTelegraph, const FDoneDelegate& Done);
END_DEFINE_SPEC(FEnemyAttackSpec)

void FEnemyAttackSpec::TelegraphGap(int32 AttackIndex, FGameplayTag ExpectedTelegraph, const FDoneDelegate& Done)
{
	TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(AttackIndex);
	TSharedRef<double> TelegraphAt = MakeShared<double>(-1.0);
	TSharedRef<double> DamageAt = MakeShared<double>(-1.0);
	RunFrames(Fixture, Done, [Fixture, TelegraphAt, DamageAt, ExpectedTelegraph]()
	{
		if (*TelegraphAt < 0.0 && Fixture->Feedback->LastPlayed == ExpectedTelegraph) { *TelegraphAt = Fixture->Now(); }
		if (*DamageAt < 0.0 && Fixture->HeroDamaged()) { *DamageAt = Fixture->Now(); }
		return *DamageAt < 0.0;
	}, [this, Fixture, TelegraphAt, DamageAt, ExpectedTelegraph]()
	{
		const float Min = UGameTuningSettings::Get()->MinEnemyTelegraphTime;
		TestTrue(FString::Printf(TEXT("%s played"), *ExpectedTelegraph.ToString()), *TelegraphAt >= 0.0);
		TestTrue("Hero took damage", *DamageAt >= 0.0);
		TestTrue(FString::Printf(TEXT("Telegraph precedes damage by %.3f s >= %.2f s"), *DamageAt - *TelegraphAt, Min),
			*TelegraphAt >= 0.0 && *DamageAt - *TelegraphAt >= Min - FrameTime);
		TestEqual("Damage from the DA", Fixture->HeroStartHealth - Fixture->Hero->GetHealthComponent()->GetCurrentHealth(), Fixture->Definition->Attacks[0].Damage);
	});
}

void FEnemyAttackSpec::Define()
{
	LatentIt("light: telegraph precedes damage by at least the minimum", [this](const FDoneDelegate& Done)
	{
		TelegraphGap(0, FeedbackTags::Enemy_Telegraph, Done);
	});

	LatentIt("heavy: plays the heavy telegraph and precedes damage by at least the minimum", [this](const FDoneDelegate& Done)
	{
		TelegraphGap(1, FeedbackTags::Enemy_Telegraph_Heavy, Done);
	});

	LatentIt("tracks during the wind-up, stops turning at hit-window start, so a sideways dodge avoids the hit", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(0);
		struct FProbe { bool bSidestepped = false; bool bMovedDuringWindup = false; float WindowYaw = 0.f; float MaxDrift = 0.f; bool bWindowEnded = false; };
		TSharedRef<FProbe> Probe = MakeShared<FProbe>();
		// Dodge exactly at hit-window start: the window event fires before the first sweep of that frame.
		Fixture->Enemy->GetMeleeTraceComponent()->OnHitWindowBegin.AddLambda([Enemy = Fixture->Enemy, Hero = Fixture->Hero, Probe]()
		{
			Probe->bSidestepped = true;
			Probe->WindowYaw = Enemy->GetActorRotation().Yaw;
			Hero->SetActorLocation(FVector(0.f, 250.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		});
		RunFrames(Fixture, Done, [Fixture, Probe]()
		{
			const bool bAttacking = Fixture->Brain()->GetState() == EEnemyBrainState::Attacking;
			const bool bWindow = Fixture->Enemy->GetMeleeTraceComponent()->IsHitWindowActive();
			if (bAttacking && !Probe->bMovedDuringWindup)
			{
				// Step slightly aside during the wind-up: the enemy should turn to follow.
				Probe->bMovedDuringWindup = true;
				Fixture->Hero->SetActorLocation(FVector(190.f, 60.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
			}
			if (Probe->bSidestepped && bWindow)
			{
				Probe->MaxDrift = FMath::Max(Probe->MaxDrift, FMath::Abs(FMath::FindDeltaAngleDegrees(Probe->WindowYaw, Fixture->Enemy->GetActorRotation().Yaw)));
			}
			Probe->bWindowEnded = Probe->bSidestepped && !bWindow;
			return !Probe->bWindowEnded;
		}, [this, Fixture, Probe]()
		{
			TestTrue("Hit window opened", Probe->bSidestepped);
			TestTrue(FString::Printf(TEXT("Turned toward the hero during the wind-up (yaw %.1f)"), Probe->WindowYaw), Probe->WindowYaw > 5.f);
			TestTrue(FString::Printf(TEXT("No turning during the hit window (drift %.2f deg)"), Probe->MaxDrift), Probe->MaxDrift < 0.5f);
			TestFalse("Sideways dodge avoided the hit", Fixture->HeroDamaged());
		});
	});

	LatentIt("plays the attack out when the target dies during the wind-up, then returns to Idle", [this](const FDoneDelegate& Done)
	{
		TSharedRef<FAttackFixture> Fixture = MakeShared<FAttackFixture>(1);
		TSharedRef<int32> Phase = MakeShared<int32>(0); // 0 waiting for the attack, 1 attack running, 2 attack ended
		RunFrames(Fixture, Done, [Fixture, Phase]()
		{
			const EEnemyBrainState State = Fixture->Brain()->GetState();
			if (*Phase == 0 && State == EEnemyBrainState::Attacking)
			{
				*Phase = 1;
				Fixture->Hero->Destroy();
			}
			else if (*Phase == 1 && State != EEnemyBrainState::Attacking)
			{
				*Phase = 2;
			}
			return !(*Phase == 2 && State == EEnemyBrainState::Idle);
		}, [this, Fixture, Phase]()
		{
			TestEqual("Attack ran to its end after the target died", *Phase, 2);
			TestEqual("Back to Idle", Fixture->Brain()->GetState(), EEnemyBrainState::Idle);
			TestFalse("Montage finished", Fixture->Enemy->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying());
		});
	});
}
#endif
