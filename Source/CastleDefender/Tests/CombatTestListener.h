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

	int32 StaminaChangedCount = 0;
	float LastStaminaCurrent = 0.f;
	float LastStaminaMax = 0.f;
	int32 StaminaSpendFailedCount = 0;
	float LastFailedCost = 0.f;
	int32 StaminaDepletedCount = 0;

	UFUNCTION()
	void HandleStaminaChanged(float Current, float Max)
	{
		++StaminaChangedCount;
		LastStaminaCurrent = Current;
		LastStaminaMax = Max;
	}

	UFUNCTION()
	void HandleStaminaSpendFailed(float Cost)
	{
		++StaminaSpendFailedCount;
		LastFailedCost = Cost;
	}

	UFUNCTION()
	void HandleStaminaDepleted()
	{
		++StaminaDepletedCount;
	}
};
