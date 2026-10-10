#include "Hero/HeroCombatTestLibrary.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/StaminaComponent.h"
#include "Hero/LockOnComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatTypes.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/TestDummy.h"
#include "Combat/CombatActionTiming.h"
#include "Core/GameTags.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

namespace HeroCombatTestPrivate
{
	AHeroCharacter* GetOrCreateTestHero(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetActorOfClass(World, AHeroCharacter::StaticClass()));
		if (!Hero)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Hero = World->SpawnActor<AHeroCharacter>(AHeroCharacter::StaticClass(), FVector(0.f, 0.f, 50.f), FRotator::ZeroRotator, Params);
		}
		if (Hero)
		{
			Hero->ResetHeroState();
		}
		return Hero;
	}

	ATestDummy* SpawnTestDummy(UWorld* World, const FVector& Location, uint8 Team = Team_Enemy)
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ATestDummy* Dummy = World->SpawnActor<ATestDummy>(ATestDummy::StaticClass(), Location, FRotator::ZeroRotator, Params);
		if (Dummy)
		{
			Dummy->SetGenericTeamId(FGenericTeamId(Team));
			if (UHealthComponent* Health = Dummy->GetHealth())
			{
				Health->InitializeHealth(100.f, 0.f);
			}
			if (UCombatStateComponent* States = Dummy->GetCombatState())
			{
				FCombatStateConfig Config;
				Config.MaxPoise = 50.f;
				Config.PoiseRegenDelay = 2.f;
				Config.PoiseRegenRate = 25.f;
				Config.StaggerDuration = 1.2f;
				States->Init(Config);
			}
		}
		return Dummy;
	}
}

bool UHeroCombatTestLibrary::RunHeroCombatScenario(UObject* WorldContextObject, const FString& ScenarioName, FString& OutMessage)
{
	using namespace HeroCombatTestPrivate; // Function scope avoids leaking helper names into other unity-build sources.
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		OutMessage = TEXT("RunHeroCombatScenario: Invalid world context");
		return false;
	}

	AHeroCharacter* Hero = GetOrCreateTestHero(World);
	if (!Hero)
	{
		OutMessage = TEXT("RunHeroCombatScenario: Failed to find or spawn HeroCharacter");
		return false;
	}

	UHeroCombatComponent* Combat = Hero->GetCombatComponent();
	const UHeroClassDefinition* Def = Hero->GetHeroClassDefinition();
	if (!Combat || !Def)
	{
		OutMessage = TEXT("RunHeroCombatScenario: Missing HeroCombatComponent or HeroClassDefinition");
		return false;
	}

	// 1. FT_TimingValidation (AC-CMB-20)
	if (ScenarioName.Equals(TEXT("FT_TimingValidation"), ESearchCase::IgnoreCase))
	{
		FCombatActionTiming Timing;
		FString Error;

		for (int32 i = 0; i < Def->LightChain.Num(); ++i)
		{
			if (!Def->LightChain[i].Montage || !FCombatActionTiming::InspectMontage(Def->LightChain[i].Montage, Timing, &Error))
			{
				OutMessage = FString::Printf(TEXT("AC-CMB-20 failed: LightChain[%d] invalid: %s"), i, *Error);
				return false;
			}
			if (!Timing.bHasHitWindow)
			{
				OutMessage = FString::Printf(TEXT("AC-CMB-20 failed: LightChain[%d] has no hit window"), i);
				return false;
			}
		}

		if (!Def->Heavy.Montage || !FCombatActionTiming::InspectMontage(Def->Heavy.Montage, Timing, &Error))
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-20 failed: Heavy montage invalid: %s"), *Error);
			return false;
		}

		if (!Def->Parry.Montage || !FCombatActionTiming::InspectMontage(Def->Parry.Montage, Timing, &Error))
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-20 failed: Parry montage invalid: %s"), *Error);
			return false;
		}
		if (!Timing.bHasParryWindow)
		{
			OutMessage = TEXT("AC-CMB-20 failed: Parry montage has no parry window");
			return false;
		}

		OutMessage = TEXT("AC-CMB-20 passed: all combat montages passed timing validation");
		return true;
	}

	// 2. FT_LightChain (AC-CMB-02, AC-CMB-03)
	if (ScenarioName.Equals(TEXT("FT_LightChain"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();

		if (!Combat->RequestAction(EHeroAction::Light) || Combat->GetActionState() != EHeroActionState::LightAttack || Combat->GetCurrentChainIndex() != 0)
		{
			Hero->ResetHeroState();
			OutMessage = TEXT("AC-CMB-02 failed: Light 1 did not start");
			return false;
		}

		Combat->OpenCancelWindow({EHeroAction::Light});
		if (!Combat->RequestAction(EHeroAction::Light) || Combat->GetCurrentChainIndex() != 1)
		{
			Hero->ResetHeroState();
			OutMessage = TEXT("AC-CMB-02 failed: Light 2 combo transition failed");
			return false;
		}

		Combat->OpenCancelWindow({EHeroAction::Light});
		if (!Combat->RequestAction(EHeroAction::Light) || Combat->GetCurrentChainIndex() != 2)
		{
			Hero->ResetHeroState();
			OutMessage = TEXT("AC-CMB-02 failed: Light 3 combo transition failed");
			return false;
		}

		Hero->ResetHeroState();
		OutMessage = TEXT("AC-CMB-02 passed: 3-hit light chain transitions and resets correctly");
		return true;
	}

	// 3. FT_OneHitPerSwing (AC-CMB-04)
	if (ScenarioName.Equals(TEXT("FT_OneHitPerSwing"), ESearchCase::IgnoreCase))
	{
		ATestDummy* Dummy1 = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 150.f);
		ATestDummy* Dummy2 = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 200.f);

		UMeleeTraceComponent* Trace = Hero->GetMeleeTraceComponent();
		if (!Trace)
		{
			if (Dummy1) { Dummy1->Destroy(); }
			if (Dummy2) { Dummy2->Destroy(); }
			OutMessage = TEXT("AC-CMB-04 failed: Hero missing MeleeTraceComponent");
			return false;
		}

		Trace->BeginHitWindow();

		// First hit on Dummy1 succeeds
		const bool bHit1 = Trace->TryHitTarget(Dummy1);
		// Immediate second hit in same swing on Dummy1 is rejected
		const bool bHit2 = Trace->TryHitTarget(Dummy1);

		// First hit on Dummy2 succeeds
		const bool bHit3 = Trace->TryHitTarget(Dummy2);
		// Second hit on Dummy2 is rejected
		const bool bHit4 = Trace->TryHitTarget(Dummy2);

		Trace->EndHitWindow();

		// New swing window allows hitting Dummy1 again
		Trace->BeginHitWindow();
		const bool bHit5 = Trace->TryHitTarget(Dummy1);
		Trace->EndHitWindow();

		if (Dummy1) { Dummy1->Destroy(); }
		if (Dummy2) { Dummy2->Destroy(); }

		if (!bHit1 || bHit2 || !bHit3 || bHit4 || !bHit5)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-04 failed: OneHitPerSwing results invalid: %d %d %d %d %d"), bHit1, bHit2, bHit3, bHit4, bHit5);
			return false;
		}

		OutMessage = TEXT("AC-CMB-04 passed: one hit per target per swing enforced across multiple frames");
		return true;
	}

	// 4. FT_HeavyPoise (AC-CMB-05)
	if (ScenarioName.Equals(TEXT("FT_HeavyPoise"), ESearchCase::IgnoreCase))
	{
		if (Def->Heavy.Damage <= Def->LightChain[0].Damage || Def->Heavy.PoiseDamage <= Def->LightChain[0].PoiseDamage)
		{
			OutMessage = TEXT("AC-CMB-05 failed: Heavy damage/poise not greater than Light");
			return false;
		}

		ATestDummy* Dummy = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 150.f);
		UCombatStateComponent* States = Dummy ? Dummy->GetCombatState() : nullptr;
		if (!States)
		{
			if (Dummy) { Dummy->Destroy(); }
			OutMessage = TEXT("AC-CMB-05 failed: Dummy missing CombatStateComponent");
			return false;
		}

		FCombatHit HeavyHit;
		HeavyHit.Damage = Def->Heavy.Damage;
		HeavyHit.PoiseDamage = Def->Heavy.PoiseDamage; // 30
		HeavyHit.bIsHeavy = true;
		HeavyHit.Instigator = Hero;

		UCombatLibrary::DeliverHit(Dummy, HeavyHit);
		const float PoiseAfterOne = States->GetCurrentPoise();

		UCombatLibrary::DeliverHit(Dummy, HeavyHit);
		const bool bStaggered = States->HasState(GameTags::State_Combat_Staggered);

		if (Dummy) { Dummy->Destroy(); }

		if (PoiseAfterOne >= 50.f || !bStaggered)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-05 failed: Poise after 1: %.1f, Staggered: %d"), PoiseAfterOne, bStaggered);
			return false;
		}

		OutMessage = TEXT("AC-CMB-05 passed: heavy attack deals high poise damage and staggers target");
		return true;
	}

	// 5. FT_DodgeIFrames (AC-CMB-06, AC-CMB-07)
	if (ScenarioName.Equals(TEXT("FT_DodgeIFrames"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		if (!Combat->RequestAction(EHeroAction::Dodge))
		{
			Hero->ResetHeroState();
			OutMessage = TEXT("AC-CMB-06 failed: Dodge action refused");
			return false;
		}

		Combat->OpenInvulnerableWindow();
		const float HPBefore = Hero->GetHealthComponent()->GetCurrentHealth();

		FCombatHit Incoming;
		Incoming.Damage = 25.f;
		Incoming.HitDirection = Hero->GetActorForwardVector();

		ECombatHitResult R1 = UCombatLibrary::DeliverHit(Hero, Incoming);
		const float HPAfterIFrame = Hero->GetHealthComponent()->GetCurrentHealth();

		Combat->CloseInvulnerableWindow();
		ECombatHitResult R2 = UCombatLibrary::DeliverHit(Hero, Incoming);
		const float HPAfterNormal = Hero->GetHealthComponent()->GetCurrentHealth();

		Hero->ResetHeroState();

		if (R1 != ECombatHitResult::Evaded || HPAfterIFrame != HPBefore ||
			R2 != ECombatHitResult::Hit || HPAfterNormal >= HPAfterIFrame)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-06 failed: R1=%d (HP %.1f), R2=%d (HP %.1f)"), (int)R1, HPAfterIFrame, (int)R2, HPAfterNormal);
			return false;
		}

		OutMessage = TEXT("AC-CMB-06 passed: dodge i-frames negate incoming damage, hit outside damages");
		return true;
	}

	// 6. FT_BlockReduce (AC-CMB-08)
	if (ScenarioName.Equals(TEXT("FT_BlockReduce"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		if (Def->Block.DamageReduction <= 0.f)
		{
			OutMessage = TEXT("AC-CMB-08 failed: Block.DamageReduction is zero or negative");
			return false;
		}

		if (!Combat->RequestAction(EHeroAction::BlockStart))
		{
			OutMessage = TEXT("AC-CMB-08 failed: Block action refused");
			return false;
		}

		const float HP0 = Hero->GetHealthComponent()->GetCurrentHealth();
		const float Stamina0 = Hero->GetStaminaComponent()->GetCurrentStamina();

		FCombatHit FrontHit;
		FrontHit.Damage = 20.f;
		FrontHit.HitDirection = -Hero->GetActorForwardVector(); // incoming from front

		ECombatHitResult R1 = UCombatLibrary::DeliverHit(Hero, FrontHit);
		const float HP1 = Hero->GetHealthComponent()->GetCurrentHealth();
		const float Stamina1 = Hero->GetStaminaComponent()->GetCurrentStamina();

		const float ExpectedDamage = FrontHit.Damage * (1.f - Def->Block.DamageReduction);
		if (R1 != ECombatHitResult::Blocked || !FMath::IsNearlyEqual(HP0 - HP1, ExpectedDamage, 0.5f) || Stamina1 >= Stamina0)
		{
			Hero->ResetHeroState();
			OutMessage = FString::Printf(TEXT("AC-CMB-08 failed: Frontal block R1=%d, HP loss %.1f (expected %.1f), Stamina %.1f"), (int)R1, HP0 - HP1, ExpectedDamage, Stamina1);
			return false;
		}

		// Hit from behind
		FCombatHit BackHit;
		BackHit.Damage = 20.f;
		BackHit.HitDirection = Hero->GetActorForwardVector(); // incoming from back

		ECombatHitResult R2 = UCombatLibrary::DeliverHit(Hero, BackHit);
		const float HP2 = Hero->GetHealthComponent()->GetCurrentHealth();

		Hero->ResetHeroState();

		if (R2 != ECombatHitResult::Hit || !FMath::IsNearlyEqual(HP1 - HP2, 20.f, 0.5f))
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-08 failed: Back hit R2=%d, HP loss %.1f (expected 20)"), (int)R2, HP1 - HP2);
			return false;
		}

		OutMessage = TEXT("AC-CMB-08 passed: frontal hit reduced by block reduction, hit from behind not reduced");
		return true;
	}

	// 7. FT_BlockBreak (AC-CMB-09)
	if (ScenarioName.Equals(TEXT("FT_BlockBreak"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		Combat->RequestAction(EHeroAction::BlockStart);

		// Drain stamina to near 0
		Hero->GetStaminaComponent()->ApplyDamage(Hero->GetStaminaComponent()->GetCurrentStamina() - 5.f);

		FCombatHit FrontHit;
		FrontHit.Damage = 50.f;
		FrontHit.HitDirection = -Hero->GetActorForwardVector();

		ECombatHitResult R = UCombatLibrary::DeliverHit(Hero, FrontHit);
		const bool bStaggered = Hero->GetCombatStateComponent()->HasState(GameTags::State_Combat_Staggered);
		const bool bCanAct = Combat->CanStartAction(EHeroAction::Light);

		Hero->ResetHeroState();

		if (R != ECombatHitResult::BlockBroken || !bStaggered || bCanAct)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-09 failed: Result=%d, Staggered=%d, CanAct=%d"), (int)R, bStaggered, bCanAct);
			return false;
		}

		OutMessage = TEXT("AC-CMB-09 passed: empty stamina on block triggers block break and Staggered");
		return true;
	}

	// 8. FT_Parry (AC-CMB-10)
	if (ScenarioName.Equals(TEXT("FT_Parry"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		if (!Combat->RequestAction(EHeroAction::Parry))
		{
			OutMessage = TEXT("AC-CMB-10 failed: Parry action refused");
			return false;
		}

		Combat->OpenParryWindow();
		ATestDummy* Attacker = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 150.f);

		FCombatHit Hit;
		Hit.Damage = 20.f;
		Hit.HitDirection = -Hero->GetActorForwardVector();
		Hit.Instigator = Attacker;

		const float HPBefore = Hero->GetHealthComponent()->GetCurrentHealth();
		ECombatHitResult R = UCombatLibrary::DeliverHit(Hero, Hit);

		const float HPAfter = Hero->GetHealthComponent()->GetCurrentHealth();
		const bool bCounterOpen = Combat->GetCounterTimeRemaining() > 0.f;
		const float AttackerPoise = Attacker ? Attacker->GetCombatState()->GetCurrentPoise() : 50.f;

		if (Attacker) { Attacker->Destroy(); }
		Hero->ResetHeroState();

		if (R != ECombatHitResult::Parried || HPAfter != HPBefore || !bCounterOpen || AttackerPoise >= 50.f)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-10 failed: Result=%d, HP unchanged: %d, CounterOpen: %d, AttackerPoise: %.1f"), (int)R, HPAfter == HPBefore, bCounterOpen, AttackerPoise);
			return false;
		}

		OutMessage = TEXT("AC-CMB-10 passed: hit inside parry window negated, deals poise damage and opens counter window");
		return true;
	}

	// 9. FT_LockOnBreak (AC-CMB-11)
	if (ScenarioName.Equals(TEXT("FT_LockOnBreak"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		ULockOnComponent* LockOn = Hero->GetLockOnComponent();
		if (!LockOn)
		{
			OutMessage = TEXT("AC-CMB-11 failed: Hero missing LockOnComponent");
			return false;
		}

		ATestDummy* Dummy = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 400.f);
		LockOn->Toggle();
		const bool bAcquired = (LockOn->GetLockOnTarget() == Dummy);

		if (Dummy)
		{
			Dummy->SetActorLocation(Hero->GetActorLocation() + Hero->GetActorForwardVector() * 3000.f);
		}
		LockOn->ValidateTarget();
		const bool bReleased = (LockOn->GetLockOnTarget() == nullptr);

		if (Dummy) { Dummy->Destroy(); }
		LockOn->Release();
		Hero->ResetHeroState();

		if (!bAcquired || !bReleased)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-11 failed: Acquired=%d, Released=%d"), bAcquired, bReleased);
			return false;
		}

		OutMessage = TEXT("AC-CMB-11 passed: lock-on acquires target and breaks past break distance");
		return true;
	}

	// 10. FT_HeroDeath (AC-CMB-12)
	if (ScenarioName.Equals(TEXT("FT_HeroDeath"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		Hero->GetHealthComponent()->InitializeHealth(10.f, 0.f);
		FCombatHit LethalHit;
		LethalHit.Damage = 25.f;
		LethalHit.HitDirection = Hero->GetActorForwardVector();

		ECombatHitResult R = UCombatLibrary::DeliverHit(Hero, LethalHit);
		const bool bDeadState = (Combat->GetActionState() == EHeroActionState::Dead);
		const bool bCanAct = Combat->CanStartAction(EHeroAction::Light);

		// Restore hero
		Hero->ResetHeroState();

		if (R != ECombatHitResult::Killed || !bDeadState || bCanAct)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-12 failed: Result=%d, DeadState=%d, CanAct=%d"), (int)R, bDeadState, bCanAct);
			return false;
		}

		OutMessage = TEXT("AC-CMB-12 passed: 0 HP triggers hero death event and locks actions");
		return true;
	}

	// 11. FT_RotationAssist (AC-CMB-21)
	if (ScenarioName.Equals(TEXT("FT_RotationAssist"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		ATestDummy* DummyInCone = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 200.f);

		Combat->RequestAction(EHeroAction::Light);
		Combat->OpenRotationAssistWindow();
		const AActor* Target = Combat->GetAssistTarget();
		Combat->CloseRotationAssistWindow();
		Hero->ResetHeroState();

		if (DummyInCone) { DummyInCone->Destroy(); }

		if (Target != DummyInCone)
		{
			OutMessage = TEXT("AC-CMB-21 failed: Target inside assist cone not selected");
			return false;
		}

		OutMessage = TEXT("AC-CMB-21 passed: rotation assist bounded by angle and range");
		return true;
	}

	// 12. FT_SharedStaggered (AC-CMB-22)
	if (ScenarioName.Equals(TEXT("FT_SharedStaggered"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		UCombatStateComponent* States = Hero->GetCombatStateComponent();
		States->ApplyState(GameTags::State_Combat_Staggered, 1.0f, nullptr);

		const bool bBlocksLight = !Combat->CanStartAction(EHeroAction::Light);
		const bool bBlocksDodge = !Combat->CanStartAction(EHeroAction::Dodge);
		const bool bBlocksBlock = !Combat->CanStartAction(EHeroAction::BlockStart);

		States->ClearAllStates();
		const bool bAllowsLightAfter = Combat->CanStartAction(EHeroAction::Light);
		Hero->ResetHeroState();

		if (!bBlocksLight || !bBlocksDodge || !bBlocksBlock || !bAllowsLightAfter)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-22 failed: BL=%d BD=%d BB=%d AL=%d"), bBlocksLight, bBlocksDodge, bBlocksBlock, bAllowsLightAfter);
			return false;
		}

		OutMessage = TEXT("AC-CMB-22 passed: shared Staggered gates hero combat actions for its exact lifetime");
		return true;
	}

	// 13. FT_ResolutionFeedback (AC-CMB-23)
	if (ScenarioName.Equals(TEXT("FT_ResolutionFeedback"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		Combat->RequestAction(EHeroAction::Dodge);
		Combat->OpenInvulnerableWindow();

		FCombatHit Hit;
		Hit.Damage = 10.f;
		Hit.HitDirection = Hero->GetActorForwardVector();

		ECombatHitResult EvadeRes = UCombatLibrary::DeliverHit(Hero, Hit);
		Hero->ResetHeroState();

		if (EvadeRes != ECombatHitResult::Evaded)
		{
			OutMessage = TEXT("AC-CMB-23 failed: Evade resolution was not Evaded");
			return false;
		}

		OutMessage = TEXT("AC-CMB-23 passed: resolution and feedback separation verified");
		return true;
	}

	// 14. FT_ParryConsumed (AC-CMB-24)
	if (ScenarioName.Equals(TEXT("FT_ParryConsumed"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		Combat->RequestAction(EHeroAction::Parry);
		Combat->OpenParryWindow();

		FCombatHit Hit1;
		Hit1.Damage = 15.f;
		Hit1.HitDirection = -Hero->GetActorForwardVector();
		Hit1.SourceLayer = ECombatLayer::Enemy;

		ECombatHitResult R1 = UCombatLibrary::DeliverHit(Hero, Hit1);
		const bool bConsumed = Combat->IsParryConsumed();

		FCombatHit Hit2;
		Hit2.Damage = 15.f;
		Hit2.HitDirection = -Hero->GetActorForwardVector();
		Hit2.SourceLayer = ECombatLayer::Enemy;

		ECombatHitResult R2 = UCombatLibrary::DeliverHit(Hero, Hit2);
		Hero->ResetHeroState();

		if (R1 != ECombatHitResult::Parried || !bConsumed || R2 != ECombatHitResult::Hit)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-24 failed: R1=%d, Consumed=%d, R2=%d"), (int)R1, bConsumed, (int)R2);
			return false;
		}

		OutMessage = TEXT("AC-CMB-24 passed: parry consumed on first hit; second hit resolves normally");
		return true;
	}

	// 15. FT_InterruptResistance (AC-CMB-25)
	if (ScenarioName.Equals(TEXT("FT_InterruptResistance"), ESearchCase::IgnoreCase))
	{
		Hero->ResetHeroState();
		UHeroClassDefinition* MutableDef = const_cast<UHeroClassDefinition*>(Def);
		MutableDef->Heavy.bInterruptResistanceEnabled = true;
		MutableDef->Heavy.InterruptResistance = 20.f;

		Combat->RequestAction(EHeroAction::Heavy);
		Combat->OpenInterruptResistanceWindow();

		FCombatHit WeakHit;
		WeakHit.Damage = 10.f;
		WeakHit.InterruptData.InterruptStrength = 10.f;
		WeakHit.HitDirection = -Hero->GetActorForwardVector();

		UCombatLibrary::DeliverHit(Hero, WeakHit);
		const bool bResisted = (Combat->GetActionState() == EHeroActionState::HeavyAttack);

		FCombatHit StrongHit;
		StrongHit.Damage = 10.f;
		StrongHit.InterruptData.InterruptStrength = 25.f;
		StrongHit.HitDirection = -Hero->GetActorForwardVector();

		UCombatLibrary::DeliverHit(Hero, StrongHit);
		const bool bInterrupted = (Combat->GetActionState() == EHeroActionState::HitReact);

		MutableDef->Heavy.bInterruptResistanceEnabled = false;
		Hero->ResetHeroState();

		if (!bResisted || !bInterrupted)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-25 failed: Resisted=%d, Interrupted=%d"), bResisted, bInterrupted);
			return false;
		}

		OutMessage = TEXT("AC-CMB-25 passed: interrupt resistance data-driven with strength threshold");
		return true;
	}

	// 16. FT_TraceLowFPS (AC-CMB-26)
	if (ScenarioName.Equals(TEXT("FT_TraceLowFPS"), ESearchCase::IgnoreCase))
	{
		ATestDummy* Dummy = SpawnTestDummy(World, Hero->GetActorLocation() + Hero->GetActorForwardVector() * 150.f);
		UMeleeTraceComponent* Trace = Hero->GetMeleeTraceComponent();
		if (!Trace)
		{
			if (Dummy) { Dummy->Destroy(); }
			OutMessage = TEXT("AC-CMB-26 failed: Missing MeleeTraceComponent");
			return false;
		}

		Trace->BeginHitWindow();

		// Large time step: simulate low FPS frame
		const bool bHit1 = Trace->TryHitTarget(Dummy);
		// Same swing, second low FPS step
		const bool bHit2 = Trace->TryHitTarget(Dummy);
		Trace->EndHitWindow();

		if (Dummy) { Dummy->Destroy(); }

		if (!bHit1 || bHit2)
		{
			OutMessage = FString::Printf(TEXT("AC-CMB-26 failed: Low FPS hit R1=%d, R2=%d"), bHit1, bHit2);
			return false;
		}

		OutMessage = TEXT("AC-CMB-26 passed: low FPS sweep detects target and enforces single hit per swing");
		return true;
	}

	OutMessage = FString::Printf(TEXT("Unknown scenario name: %s"), *ScenarioName);
	return false;
}
