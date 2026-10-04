#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/CombatTypes.h"
#include "CombatHitInterceptor.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatHitInterceptor : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface allowing target components (e.g. UHeroCombatComponent) or actors
 * to intercept incoming combat hits before health damage is applied (T-CMB-04).
 * Handles defensive mechanics like Evaded (dodge i-frames), Parried, and Blocked.
 */
class CASTLEDEFENDER_API ICombatHitInterceptor
{
	GENERATED_BODY()

public:
	/**
	 * Intercepts an incoming hit.
	 * Can modify the hit (e.g. damage reduction) or reject it (Evaded, Parried).
	 */
	virtual ECombatHitResult InterceptHit(FCombatHit& Hit) = 0;

	/**
	 * Notification dispatched when combat resolution concludes for this participant.
	 */
	virtual void NotifyCombatResolved(const FCombatResolutionEvent& Event) {}
};
