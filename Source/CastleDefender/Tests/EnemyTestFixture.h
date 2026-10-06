#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enemy/EnemyArchetypeDefinition.h"
#include "Enemy/EnemyCharacter.h"

/** Test world set up and torn down like FTestWorldWrapper: an engine context (destroying actors in a context-less world warns), EndPlay and GC on teardown. */
struct FEnemyTestWorld
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FEnemyTestWorld() { GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World); }
	~FEnemyTestWorld()
	{
		// End play for every actor that began it. Engine actors such as Water's ABuoyancyManager register physics-solver
		// callbacks in BeginPlay and remove them only in EndPlay; skipping it crashes the next garbage collection.
		World->BeginTearingDown();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->HasActorBegunPlay()) { It->RouteEndPlay(EEndPlayReason::Quit); }
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		// As FTestWorldWrapper: collect now so a destroyed world never survives into the next test's map load.
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
};

/** Valid archetype with fixed fixture numbers, not live gameplay tuning. */
inline UEnemyArchetypeDefinition* MakeTestEnemyDefinition(UObject* Outer)
{
	UEnemyArchetypeDefinition* Definition = NewObject<UEnemyArchetypeDefinition>(Outer);
	Definition->DisplayName = FText::FromString(TEXT("Enemy Fixture"));
	Definition->EnemyClass = AEnemyCharacter::StaticClass();
	Definition->MaxHealth = 100.f;
	Definition->WalkSpeed = 300.f;
	Definition->CombatState.MaxPoise = 50.f;
	FEnemyAttackDefinition Attack;
	Attack.Montage = NewObject<UAnimMontage>(Definition);
	Attack.Range = 150.f;
	Attack.Damage = 10.f;
	Definition->Attacks.Add(Attack);
	return Definition;
}
#endif
