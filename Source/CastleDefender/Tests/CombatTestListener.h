#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/CombatTypes.h"
#include "Combat/HealthComponent.h"
#include "CombatTestListener.generated.h"

/** Test helper: counts combat delegate broadcasts (dynamic delegates need a UFUNCTION target). */
UCLASS(Transient)
class UCombatTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 DeathCount = 0;
	UHealthComponent* Health = nullptr;
	float NestedDamage = 0.f;
	float NestedApplied = -1.f;
	bool bDeadDuringDamage = false;
	bool bNestedHitSent = false;

	UFUNCTION()
	void HandleDamage(const FCombatHit& Hit, float RemainingHealth)
	{
		if (!bNestedHitSent)
		{
			bNestedHitSent = true;
			bDeadDuringDamage = Health->IsDead();
			FCombatHit NestedHit;
			NestedHit.Damage = NestedDamage;
			NestedApplied = Health->ApplyHit(NestedHit);
		}
	}

	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit) { ++DeathCount; }
};
