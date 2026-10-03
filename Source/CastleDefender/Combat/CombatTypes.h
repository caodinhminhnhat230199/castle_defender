#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CombatTypes.generated.h"

/** Who produced a hit. Lets Hero, Army and Tower create openings for each other (D-05). */
UENUM(BlueprintType)
enum class ECombatLayer : uint8
{
	Hero,
	Army,
	Tower,
	Enemy,
	Environment
};

/** One hit. Built by the attacker, delivered through UCombatLibrary::DeliverHit (T-CMB-04). */
USTRUCT(BlueprintType)
struct FCombatHit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ECombatLayer SourceLayer = ECombatLayer::Environment;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float Damage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float PoiseDamage = 0.f;

	/** Damage.Physical in the prototype. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag DamageType;

	/** States applied on hit, e.g. State.Combat.ArmorBroken. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer AppliedStates;

	/** Seconds; 0 = the state's default duration. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float StateDuration = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsHeavy = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsParryCounter = false;
};

/** FGenericTeamId values. No custom faction system. */
inline constexpr uint8 Team_Player = 0;
inline constexpr uint8 Team_Enemy = 1;

/** True when both actors have a team (IGenericTeamAgentInterface) and the teams differ. */
CASTLEDEFENDER_API bool AreHostile(const AActor* A, const AActor* B);
