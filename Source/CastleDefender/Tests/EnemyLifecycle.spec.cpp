#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Enemy/EnemyCharacter.h"
#include "Tests/EnemyLifecycleTestListener.h"
#include "Tests/EnemyTestFixture.h"
#include "Combat/CombatLibrary.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Core/GameTags.h"
#include "Engine/World.h"
#include "Misc/DataValidation.h"

namespace
{
	struct FEnemyLifecycleFixture : FEnemyTestWorld
	{
		AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>();
		UEnemyArchetypeDefinition* Definition = MakeTestEnemyDefinition(Enemy);
		UEnemyLifecycleTestListener* Listener = NewObject<UEnemyLifecycleTestListener>();

		FEnemyLifecycleFixture()
		{
			Enemy->InitFromSpawn(Definition, FEnemySpawnParams());
			Enemy->OnEnemyRemoved.AddDynamic(Listener, &UEnemyLifecycleTestListener::HandleRemoved);
		}
		void BeginPlay()
		{
			World->InitializeActorsForPlay(FURL());
			Enemy->DispatchBeginPlay();
		}
	};
}

BEGIN_DEFINE_SPEC(FEnemyLifecycleSpec, "CastleDefender.Enemy.Lifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FEnemyLifecycleSpec)

void FEnemyLifecycleSpec::Define()
{
	It("initializes health, poise and movement from data and copies attacks without mutating it", [this]()
	{
		FEnemyLifecycleFixture Fixture;
		Fixture.Definition->MaxHealth = 125.f;
		Fixture.Definition->CombatState.MaxPoise = 75.f;
		Fixture.BeginPlay();
		TestEqual("Health from DA", Fixture.Enemy->GetHealthComponent()->GetMaxHealth(), 125.f);
		TestEqual("Poise from DA", Fixture.Enemy->GetCombatStateComponent()->GetMaxPoise(), 75.f);
		TestEqual("Speed from DA", Fixture.Enemy->GetCharacterMovement()->MaxWalkSpeed, Fixture.Definition->WalkSpeed);
		TestEqual("Enemy team", Fixture.Enemy->GetGenericTeamId().GetId(), Team_Enemy);
		FEnemyRuntimeParams Copy = Fixture.Enemy->GetRuntimeParams();
		Copy.Attacks[0].Damage = 99.f;
		TestEqual("DA unchanged", Fixture.Definition->Attacks[0].Damage, 10.f);
	});

	It("reports killed once, removes pawn collision and schedules the authored lifespan", [this]()
	{
		FEnemyLifecycleFixture Fixture;
		Fixture.BeginPlay();
		FCombatHit Hit;
		Hit.Damage = Fixture.Definition->MaxHealth;
		TestEqual("Killed through shared contract", UCombatLibrary::DeliverHit(Fixture.Enemy, Hit), ECombatHitResult::Killed);
		TestEqual("One report", Fixture.Listener->Count, 1);
		TestEqual("Killed reason", Fixture.Listener->LastReason, EEnemyRemovedReason::Killed);
		TestEqual("Capsule ignores hero", Fixture.Enemy->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn), ECR_Ignore);
		TestEqual("Cannot move", Fixture.Enemy->GetCharacterMovement()->MovementMode, static_cast<TEnumAsByte<EMovementMode>>(MOVE_None));
		TestEqual("Despawn delay", Fixture.Enemy->GetLifeSpan(), Fixture.Definition->DespawnDelay, 0.01f);
		Fixture.Enemy->Despawn();
		TestEqual("Death plus despawn still one", Fixture.Listener->Count, 1);
	});

	It("reports explicit despawn, out of world and fallback destruction exactly once", [this]()
	{
		{
			FEnemyLifecycleFixture Fixture;
			Fixture.BeginPlay();
			Fixture.Enemy->Despawn();
			TestEqual("Despawn count", Fixture.Listener->Count, 1);
			TestEqual("Despawn reason", Fixture.Listener->LastReason, EEnemyRemovedReason::Despawned);
		}
		{
			FEnemyLifecycleFixture Fixture;
			Fixture.BeginPlay();
			Fixture.Enemy->FellOutOfWorld(*GetDefault<UDamageType>());
			TestEqual("Out of world count", Fixture.Listener->Count, 1);
			TestEqual("Out of world reason", Fixture.Listener->LastReason, EEnemyRemovedReason::OutOfWorld);
		}
		{
			FEnemyLifecycleFixture Fixture;
			Fixture.BeginPlay();
			Fixture.Enemy->Destroy();
			TestEqual("Fallback count", Fixture.Listener->Count, 1);
			TestEqual("Fallback reason", Fixture.Listener->LastReason, EEnemyRemovedReason::Despawned);
		}
	});

	It("guards reentrant death observers and destroys immediately when delay is zero", [this]()
	{
		FEnemyLifecycleFixture Fixture;
		Fixture.Definition->DespawnDelay = 0.f;
		Fixture.Listener->bDespawnOnRemoval = true;
		Fixture.BeginPlay();
		FCombatHit Hit;
		Hit.Damage = Fixture.Definition->MaxHealth;
		UCombatLibrary::DeliverHit(Fixture.Enemy, Hit);
		TestEqual("One nested report", Fixture.Listener->Count, 1);
		TestTrue("Flag committed before callback", Fixture.Listener->bSawTerminalState);
		TestTrue("Destroyed", Fixture.Enemy->IsActorBeingDestroyed());
	});

	It("destroys a zero-delay corpse without relying on any removal observer", [this]()
	{
		FEnemyLifecycleFixture Fixture;
		Fixture.Definition->DespawnDelay = 0.f;
		Fixture.BeginPlay();
		FCombatHit Hit;
		Hit.Damage = Fixture.Definition->MaxHealth;
		UCombatLibrary::DeliverHit(Fixture.Enemy, Hit);
		TestEqual("One report", Fixture.Listener->Count, 1);
		TestTrue("Zero delay destroys", Fixture.Enemy->IsActorBeingDestroyed());
	});

	It("rejects an empty attack list in editor validation and a missing runtime archetype", [this]()
	{
		FEnemyLifecycleFixture Fixture;
		Fixture.Definition->Attacks.Reset();
		FDataValidationContext Context;
		TestEqual("No attacks invalid", Fixture.Definition->IsDataValid(Context), EDataValidationResult::Invalid);
		Fixture.Enemy->InitFromSpawn(nullptr, FEnemySpawnParams());
		AddExpectedError(TEXT("invalid enemy archetype"), EAutomationExpectedErrorFlags::Contains, 1);
		Fixture.BeginPlay();
		TestEqual("Bad spawn reports once", Fixture.Listener->Count, 1);
		TestEqual("Bad spawn reason", Fixture.Listener->LastReason, EEnemyRemovedReason::Despawned);
	});
}
#endif
