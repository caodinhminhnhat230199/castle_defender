#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Enemy/EnemyArchetypeDefinition.h"
#include "Enemy/EnemyCharacter.h"

/** Test world with an engine context: destroying actors in a context-less world logs a warning (as in FTestWorldWrapper). */
struct FEnemyTestWorld
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FEnemyTestWorld() { GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World); }
	~FEnemyTestWorld()
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
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
