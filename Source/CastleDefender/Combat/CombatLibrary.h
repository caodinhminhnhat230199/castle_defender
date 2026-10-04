#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/CombatTypes.h"
#include "CombatLibrary.generated.h"

class AActor;
class ICombatHitInterceptor;

/**
 * Combat library providing the shared damage delivery pipeline and geometric queries.
 *
 * ARCHITECTURAL CONTRACT (D-05, R-CMB-50):
 * Every hit in the game MUST be delivered through UCombatLibrary::DeliverHit.
 * Enemy attacks (ENM), squad soldiers (SQD), and defensive towers (DEF)
 * must NEVER call UHealthComponent::ApplyHit directly.
 * DeliverHit is the single pipeline responsible for hostility validation, defensive interception
 * (evade i-frames, parry, block), armor mitigation, poise damage, status effect propagation,
 * and combat resolution telemetry.
 */
UCLASS()
class CASTLEDEFENDER_API UCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Delivers a combat hit from an attacker to a target.
	 * Sole authoritative entry point for damage application in the game.
	 *
	 * @param Target The actor receiving the hit.
	 * @param Hit Specification of damage, poise, direction, and tags.
	 * @return The final outcome of the hit (Ignored, Evaded, Parried, Blocked, BlockBroken, Hit, Killed).
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static ECombatHitResult DeliverHit(AActor* Target, const FCombatHit& Hit);

	/**
	 * Tests whether AttackerLocation lies within the specified front arc angle of Defender.
	 * Evaluated in 2D horizontal plane (yaw).
	 *
	 * @param Defender The defending actor whose forward vector defines the center of the arc.
	 * @param AttackerLocation The world location of the incoming attack or attacker.
	 * @param ArcDegrees Total angular width of the arc in degrees (e.g. 140 for Warlord block).
	 * @return True if AttackerLocation is within ArcDegrees/2 of Defender's forward facing.
	 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool IsInFrontArc(const AActor* Defender, const FVector& AttackerLocation, float ArcDegrees);

private:
	static ICombatHitInterceptor* FindHitInterceptor(AActor* Actor);
	static void DispatchCombatResolution(const FCombatResolutionEvent& Event);
};
