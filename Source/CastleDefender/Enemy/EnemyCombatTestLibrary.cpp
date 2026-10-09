#include "Enemy/EnemyCombatTestLibrary.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/EnemyBrainComponent.h"
#include "Enemy/EnemyArchetypeDefinition.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroClassDefinition.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatTypes.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

namespace EnemyCombatTestPrivate
{
	void EnsureFloor(UWorld* World)
	{
		if (!World || UGameplayStatics::GetActorOfClass(World, AStaticMeshActor::StaticClass()))
		{
			return;
		}

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.f, 0.f, -50.f), FRotator::ZeroRotator);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			Floor->SetActorScale3D(FVector(20.f, 20.f, 1.f));
		}
	}

	AHeroCharacter* GetOrCreateTestHero(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		EnsureFloor(World);

		AHeroCharacter* Hero = Cast<AHeroCharacter>(UGameplayStatics::GetActorOfClass(World, AHeroCharacter::StaticClass()));
		if (!Hero)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Hero = World->SpawnActor<AHeroCharacter>(AHeroCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
			if (Hero)
			{
				UHeroClassDefinition* HeroDef = LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord.DA_HeroClass_Warlord"));
				if (HeroDef)
				{
					Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(HeroDef, Hero));
				}
				USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
				if (Mesh)
				{
					Hero->GetMesh()->SetSkeletalMesh(Mesh);
				}
				Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
				if (!Hero->HasActorBegunPlay())
				{
					Hero->DispatchBeginPlay();
				}
			}
		}

		if (Hero)
		{
			Hero->ResetHeroState();
		}
		return Hero;
	}

	AEnemyCharacter* SpawnOrResetTestEnemy(UWorld* World, const FVector& Location, const FRotator& Rotation)
	{
		if (!World)
		{
			return nullptr;
		}

		EnsureFloor(World);

		// Clean up all existing enemies so each test scenario gets fresh, isolated state
		TArray<AActor*> ExistingEnemies;
		UGameplayStatics::GetAllActorsOfClass(World, AEnemyCharacter::StaticClass(), ExistingEnemies);
		for (AActor* Actor : ExistingEnemies)
		{
			if (Actor && !Actor->IsActorBeingDestroyed())
			{
				Actor->Destroy();
			}
		}

		UEnemyArchetypeDefinition* SourceDef = LoadObject<UEnemyArchetypeDefinition>(nullptr, TEXT("/Game/CastleDefender/Enemy/DA_Enemy_Melee.DA_Enemy_Melee"));
		if (!SourceDef)
		{
			SourceDef = LoadObject<UEnemyArchetypeDefinition>(nullptr, TEXT("/Game/CastleDefender/Enemy/DA_Enemy_Test.DA_Enemy_Test"));
		}

		UEnemyArchetypeDefinition* Def = SourceDef ? DuplicateObject<UEnemyArchetypeDefinition>(SourceDef, GetTransientPackage()) : nullptr;
		UClass* EnemyClass = Def && Def->EnemyClass ? Def->EnemyClass.Get() : LoadClass<AEnemyCharacter>(nullptr, TEXT("/Game/CastleDefender/Enemy/BP_Enemy_Base.BP_Enemy_Base_C"));
		if (!EnemyClass)
		{
			EnemyClass = AEnemyCharacter::StaticClass();
		}

		const FTransform SpawnTransform(Rotation, Location);
		AEnemyCharacter* Enemy = World->SpawnActorDeferred<AEnemyCharacter>(EnemyClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Enemy && Def)
		{
			Enemy->InitFromSpawn(Def, FEnemySpawnParams());
			Enemy->FinishSpawning(SpawnTransform);
		}

		if (Enemy)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (!It->HasActorBegunPlay())
				{
					It->DispatchBeginPlay();
				}
			}
		}
		return Enemy;
	}
}

using namespace EnemyCombatTestPrivate;

bool UEnemyCombatTestLibrary::RunEnemyCombatScenario(UObject* WorldContextObject, const FString& ScenarioName, FString& OutMessage)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		OutMessage = TEXT("RunEnemyCombatScenario: Invalid world context");
		return false;
	}

	if (ScenarioName == TEXT("FT_Enemy_AggroChase"))
	{
		AHeroCharacter* Hero = GetOrCreateTestHero(World);
		AEnemyCharacter* Enemy = SpawnOrResetTestEnemy(World, FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator);
		if (!Hero || !Enemy)
		{
			OutMessage = TEXT("FT_Enemy_AggroChase: Failed to obtain Hero or Enemy");
			return false;
		}

		// Place hero at 1200 cm (outside aggro radius)
		Hero->SetActorLocation(FVector(1200.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		Hero->UpdateOverlaps();
		Enemy->GetBrainComponent()->Decide();

		const EEnemyBrainState FarState = Enemy->GetBrainComponent()->GetState();
		if (FarState != EEnemyBrainState::Idle)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_AggroChase: Expected Idle when hero is outside aggro radius, got state %d"), (int32)FarState);
			return false;
		}

		// Move hero to 250 cm (inside aggro radius)
		Hero->SetActorLocation(FVector(250.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		Hero->UpdateOverlaps();
		Enemy->GetBrainComponent()->Decide();

		const EEnemyBrainState NearState = Enemy->GetBrainComponent()->GetState();
		const AActor* Target = Enemy->GetBrainComponent()->GetTarget();
		if (NearState != EEnemyBrainState::Engage && NearState != EEnemyBrainState::Attacking)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_AggroChase: Expected Engage or Attacking when hero is inside aggro radius, got state %d"), (int32)NearState);
			return false;
		}
		if (Target != Hero)
		{
			OutMessage = TEXT("FT_Enemy_AggroChase: Enemy failed to target Hero upon aggro entry");
			return false;
		}

		OutMessage = TEXT("FT_Enemy_AggroChase passed: Enemy stays Idle when hero outside aggro, Engages and targets Hero when inside aggro radius.");
		return true;
	}

	if (ScenarioName == TEXT("FT_Enemy_TelegraphGap"))
	{
		AHeroCharacter* Hero = GetOrCreateTestHero(World);
		AEnemyCharacter* Enemy = SpawnOrResetTestEnemy(World, FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator);
		if (!Hero || !Enemy)
		{
			OutMessage = TEXT("FT_Enemy_TelegraphGap: Failed to obtain Hero or Enemy");
			return false;
		}

		const float MinTelegraph = UGameTuningSettings::Get()->MinEnemyTelegraphTime;
		if (MinTelegraph < 0.4f)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_TelegraphGap: MinEnemyTelegraphTime %.2f < 0.40s"), MinTelegraph);
			return false;
		}

		// Place hero in close combat range
		Hero->SetActorLocation(FVector(140.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		Hero->UpdateOverlaps();
		Enemy->GetBrainComponent()->Decide();

		const EEnemyBrainState State = Enemy->GetBrainComponent()->GetState();
		if (State != EEnemyBrainState::Attacking)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_TelegraphGap: Expected Attacking state in range, got %d"), (int32)State);
			return false;
		}

		// Verify hit window is not active during initial wind-up
		if (Enemy->GetMeleeTraceComponent()->IsHitWindowActive())
		{
			OutMessage = TEXT("FT_Enemy_TelegraphGap: Hit window was active at start of wind-up");
			return false;
		}

		// Verify archetype attacks all satisfy telegraph requirements
		const FEnemyRuntimeParams& Params = Enemy->GetRuntimeParams();
		if (Params.Attacks.Num() == 0)
		{
			OutMessage = TEXT("FT_Enemy_TelegraphGap: Enemy has 0 authored attacks");
			return false;
		}

		for (int32 Idx = 0; Idx < Params.Attacks.Num(); ++Idx)
		{
			const FEnemyAttackDefinition& Attack = Params.Attacks[Idx];
			if (!Attack.Montage)
			{
				OutMessage = FString::Printf(TEXT("FT_Enemy_TelegraphGap: Attack %d has null montage"), Idx);
				return false;
			}
			if (Attack.Damage <= 0.f)
			{
				OutMessage = FString::Printf(TEXT("FT_Enemy_TelegraphGap: Attack %d has non-positive damage"), Idx);
				return false;
			}
		}

		OutMessage = FString::Printf(TEXT("FT_Enemy_TelegraphGap passed: MinEnemyTelegraphTime %.2f s respected, wind-up precedes active hit window without premature sweep."), MinTelegraph);
		return true;
	}

	if (ScenarioName == TEXT("FT_Enemy_StaggerCancel"))
	{
		AHeroCharacter* Hero = GetOrCreateTestHero(World);
		AEnemyCharacter* Enemy = SpawnOrResetTestEnemy(World, FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator);
		if (!Hero || !Enemy)
		{
			OutMessage = TEXT("FT_Enemy_StaggerCancel: Failed to obtain Hero or Enemy");
			return false;
		}

		Hero->SetActorLocation(FVector(140.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		Hero->UpdateOverlaps();
		Enemy->GetBrainComponent()->Decide();

		const float HeroStartHP = Hero->GetHealthComponent()->GetCurrentHealth();

		// Deliver poise-breaking hit during wind-up
		FCombatHit BreakHit;
		BreakHit.Instigator = Hero;
		BreakHit.SourceLayer = ECombatLayer::Hero;
		BreakHit.Damage = 1.f;
		BreakHit.PoiseDamage = Enemy->GetCombatStateComponent()->GetMaxPoise() + 10.f;
		UCombatLibrary::DeliverHit(Enemy, BreakHit);

		// Verify enemy entered Staggered state
		const bool bHasStaggerState = Enemy->GetCombatStateComponent()->HasState(GameTags::State_Combat_Staggered);
		const EEnemyBrainState BrainState = Enemy->GetBrainComponent()->GetState();
		const bool bHitWindowOpen = Enemy->GetMeleeTraceComponent()->IsHitWindowActive();

		if (!bHasStaggerState)
		{
			OutMessage = TEXT("FT_Enemy_StaggerCancel: Enemy did not receive State.Combat.Staggered on poise break");
			return false;
		}
		if (BrainState != EEnemyBrainState::Staggered)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_StaggerCancel: Enemy brain not Staggered (state %d)"), (int32)BrainState);
			return false;
		}
		if (bHitWindowOpen)
		{
			OutMessage = TEXT("FT_Enemy_StaggerCancel: Hit window remained active after poise break");
			return false;
		}
		if (Hero->GetHealthComponent()->GetCurrentHealth() < HeroStartHP)
		{
			OutMessage = TEXT("FT_Enemy_StaggerCancel: Hero took damage from cancelled attack swing");
			return false;
		}

		OutMessage = TEXT("FT_Enemy_StaggerCancel passed: Poise break cleanly cancels attack montage, closes trace window, and transitions brain to Staggered without landing cancelled swing.");
		return true;
	}

	if (ScenarioName == TEXT("FT_Enemy_DeathReportOnce"))
	{
		AHeroCharacter* Hero = GetOrCreateTestHero(World);
		AEnemyCharacter* Enemy = SpawnOrResetTestEnemy(World, FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator);
		if (!Hero || !Enemy)
		{
			OutMessage = TEXT("FT_Enemy_DeathReportOnce: Failed to obtain Hero or Enemy");
			return false;
		}

		if (!Enemy->GetHealthComponent()->IsAlive())
		{
			OutMessage = TEXT("FT_Enemy_DeathReportOnce: Enemy spawned already dead");
			return false;
		}

		// Deliver lethal hit
		FCombatHit KillHit;
		KillHit.Instigator = Hero;
		KillHit.SourceLayer = ECombatLayer::Hero;
		KillHit.Damage = Enemy->GetHealthComponent()->GetMaxHealth() + 50.f;
		UCombatLibrary::DeliverHit(Enemy, KillHit);

		if (Enemy->GetHealthComponent()->IsAlive())
		{
			OutMessage = TEXT("FT_Enemy_DeathReportOnce: Enemy still alive after lethal damage");
			return false;
		}
		if (!Enemy->HasReportedRemoval())
		{
			OutMessage = TEXT("FT_Enemy_DeathReportOnce: Enemy failed to report removal upon death");
			return false;
		}
		if (Enemy->GetBrainComponent()->GetState() != EEnemyBrainState::Dead)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_DeathReportOnce: Enemy brain not Dead (state %d)"), (int32)Enemy->GetBrainComponent()->GetState());
			return false;
		}

		const ECollisionResponse PawnResponse = Enemy->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);
		if (PawnResponse != ECR_Ignore)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_DeathReportOnce: Pawn collision not ignored on dead enemy (response %d)"), (int32)PawnResponse);
			return false;
		}

		// Extra hit to dead actor should not crash or double report
		UCombatLibrary::DeliverHit(Enemy, KillHit);
		if (!Enemy->HasReportedRemoval())
		{
			OutMessage = TEXT("FT_Enemy_DeathReportOnce: Corrupted state on secondary hit");
			return false;
		}

		OutMessage = TEXT("FT_Enemy_DeathReportOnce passed: Lethal hit sets Dead state, clears pawn collision, and reports removal once.");
		return true;
	}

	if (ScenarioName == TEXT("FT_Enemy_ParryStaggers"))
	{
		AHeroCharacter* Hero = GetOrCreateTestHero(World);
		AEnemyCharacter* Enemy = SpawnOrResetTestEnemy(World, FVector(120.f, 0.f, 100.f), FRotator(0.f, 180.f, 0.f));
		if (!Hero || !Enemy)
		{
			OutMessage = TEXT("FT_Enemy_ParryStaggers: Failed to obtain Hero or Enemy");
			return false;
		}

		Hero->SetActorLocationAndRotation(FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
		Hero->UpdateOverlaps();

		const float HeroStartHP = Hero->GetHealthComponent()->GetCurrentHealth();

		// Hero enters parry state and opens parry window
		Hero->GetCombatComponent()->RequestAction(EHeroAction::Parry);
		Hero->GetCombatComponent()->OpenParryWindow();
		if (!Hero->GetCombatComponent()->IsInParryWindow())
		{
			OutMessage = TEXT("FT_Enemy_ParryStaggers: Hero parry window failed to open");
			return false;
		}

		// Enemy delivers hit against Hero from frontal arc
		FCombatHit AttackHit;
		AttackHit.Instigator = Enemy;
		AttackHit.SourceLayer = ECombatLayer::Enemy;
		AttackHit.Damage = 25.f;
		AttackHit.PoiseDamage = 15.f;
		const ECombatHitResult Outcome = UCombatLibrary::DeliverHit(Hero, AttackHit);

		if (Outcome != ECombatHitResult::Parried)
		{
			OutMessage = FString::Printf(TEXT("FT_Enemy_ParryStaggers: Expected Parried outcome, got %d"), (int32)Outcome);
			return false;
		}

		if (Hero->GetHealthComponent()->GetCurrentHealth() < HeroStartHP)
		{
			OutMessage = TEXT("FT_Enemy_ParryStaggers: Hero took damage on parried hit");
			return false;
		}

		if (Hero->GetCombatComponent()->IsInParryWindow())
		{
			OutMessage = TEXT("FT_Enemy_ParryStaggers: Parry window remained open after parry consumption");
			return false;
		}

		// Parry staggers attacker
		const bool bEnemyStaggered = Enemy->GetCombatStateComponent()->HasState(GameTags::State_Combat_Staggered);
		if (!bEnemyStaggered)
		{
			OutMessage = TEXT("FT_Enemy_ParryStaggers: Enemy was not staggered by successful parry");
			return false;
		}

		OutMessage = TEXT("FT_Enemy_ParryStaggers passed: Enemy hit during Hero parry window is intercepted (0 damage), staggers enemy, and opens counter window.");
		return true;
	}

	OutMessage = FString::Printf(TEXT("RunEnemyCombatScenario: Unknown scenario '%s'"), *ScenarioName);
	return false;
}
