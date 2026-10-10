#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
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

/** Outcome of hit delivery through UCombatLibrary::DeliverHit (T-CMB-04). */
UENUM(BlueprintType)
enum class ECombatHitResult : uint8
{
	Ignored,
	Evaded,
	Parried,
	Blocked,
	BlockBroken,
	Hit,
	Killed
};

/** Data-defined interrupt resistance and attack strength (R-CMB-51, default off). */
USTRUCT(BlueprintType)
struct FCombatInterruptData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanInterrupt = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float InterruptStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float InterruptResistance = 0.0f;
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

	/** Physical surface returned by the attack trace; unknown callers use the row's default sound. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TEnumAsByte<EPhysicalSurface> Surface = SurfaceType_Default;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsHeavy = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsParryCounter = false;

	/** DeliverHit sets this on the health payload after interception; attackers cannot assert it. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWasBlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FCombatInterruptData InterruptData;

	/** Resolver-owned outcome metadata; attackers cannot assert perfect-dodge success. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWasPerfectDodged = false;
};

/** Telemetry / resolution record emitted by UCombatLibrary::DeliverHit (R-CMB-53). */
USTRUCT(BlueprintType)
struct FCombatResolutionEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGuid ResolutionId;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FCombatHit Hit;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ECombatHitResult Result = ECombatHitResult::Ignored;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float DamageApplied = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float PoiseDamageApplied = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsHeavy = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bTargetArmored = false;
};

/** FGenericTeamId values. No custom faction system. */
inline constexpr uint8 Team_Player = 0;
inline constexpr uint8 Team_Enemy = 1;

/** True when both actors have a team (IGenericTeamAgentInterface) and the teams differ. */
CASTLEDEFENDER_API bool AreHostile(const AActor* A, const AActor* B);
