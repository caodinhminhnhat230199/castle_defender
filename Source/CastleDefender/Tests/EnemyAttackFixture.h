#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyTestFixture.h"
#include "Tests/FeedbackTestListener.h"
#include "Animation/AnimInstance.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/Ticker.h"
#include "Core/GameTuningSettings.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Enemy/EnemyBrainComponent.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"

namespace EnemyAttackTest
{
	constexpr float FrameTime = 1.f / 60.f;

	/**
	 * Real content on a floor: BP_Enemy_Base with one DA_Enemy_Test attack at the origin facing +X, and the Warlord
	 * 2 m ahead (inside attack range). The whole world ticks, so timer → montage → hit window → trace → DeliverHit runs as in game.
	 */
	struct FAttackFixture : FEnemyTestWorld
	{
		UEnemyArchetypeDefinition* Definition = nullptr;
		AEnemyCharacter* Enemy = nullptr;
		AHeroCharacter* Hero = nullptr;
		UFeedbackTestListener* Feedback = NewObject<UFeedbackTestListener>();
		float HeroStartHealth = 0.f;

		explicit FAttackFixture(int32 AttackIndex)
		{
			AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.f, 0.f, -50.f), FRotator::ZeroRotator);
			Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			Floor->SetActorScale3D(FVector(20.f, 20.f, 1.f));

			Definition = DuplicateObject(LoadObject<UEnemyArchetypeDefinition>(nullptr, TEXT("/Game/CastleDefender/Enemy/DA_Enemy_Test.DA_Enemy_Test")), GetTransientPackage());
			Definition->Attacks = { Definition->Attacks[AttackIndex] };
			const FTransform EnemyAt(FVector(0.f, 0.f, 100.f));
			Enemy = World->SpawnActorDeferred<AEnemyCharacter>(LoadClass<AEnemyCharacter>(nullptr, TEXT("/Game/CastleDefender/Enemy/BP_Enemy_Base.BP_Enemy_Base_C")),
				EnemyAt, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Enemy->InitFromSpawn(Definition, FEnemySpawnParams());
			Enemy->FinishSpawning(EnemyAt);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Hero = World->SpawnActor<AHeroCharacter>(FVector(200.f, 0.f, 100.f), FRotator(0.f, 180.f, 0.f), Params);
			Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(
				LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
			Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
			Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());

			World->InitializeActorsForPlay(FURL());
			// Every actor, including the enemy's AI controller: its tick turns the focus into the desired rotation.
			for (TActorIterator<AActor> It(World); It; ++It) { if (!It->HasActorBegunPlay()) { It->DispatchBeginPlay(); } }
			HeroStartHealth = Hero->GetHealthComponent()->GetCurrentHealth();
			World->GetSubsystem<UFeedbackSubsystem>()->OnFeedbackPlayed.AddDynamic(Feedback, &UFeedbackTestListener::HandlePlayed);
		}

		void Tick() { World->Tick(LEVELTICK_All, FrameTime); }
		double Now() const { return World->GetTimeSeconds(); }
		UEnemyBrainComponent* Brain() const { return Enemy->GetBrainComponent(); }
		bool HeroDamaged() const { return Hero->GetHealthComponent()->GetCurrentHealth() < HeroStartHealth; }
	};

	/** Ticks the fixture once per real frame (one timer-manager tick per frame) until Step returns false or 4 s pass. */
	inline void RunFrames(TSharedRef<FAttackFixture> Fixture, const FDoneDelegate& Done, TFunction<bool()> Step, TFunction<void()> Finish)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Fixture, Done, Step, Finish](float)
		{
			Fixture->Tick();
			if (Step() && Fixture->Now() < 4.0) { return true; }
			Finish();
			Done.Execute();
			return false;
		}));
	}
}
#endif
