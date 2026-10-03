#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/CombatTypes.h"
#include "CombatTestListener.generated.h"

/** Test helper: counts combat delegate broadcasts (dynamic delegates need a UFUNCTION target). */
UCLASS(Transient)
class UCombatTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 DeathCount = 0;

	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit) { ++DeathCount; }
};
