#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTypes.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthDamagedSignature, const FCombatHit&, Hit, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthDeathSignature, const FCombatHit&, KillingHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthHealedSignature, float, Amount, float, NewHealth);

/** Health for every damageable actor (D-05). Hits arrive through UCombatLibrary::DeliverHit only. */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/** Called by the owner with values from its definition asset. Resets health to full. */
	void InitializeHealth(float InMaxHealth, float InBaseArmor);

	/**
	 * Applies damage and returns the amount applied. Fires OnDeath once.
	 * Uses the target's armor and shared Armor Broken state for every source layer.
	 * Ignored while the owner cannot be damaged (the engine `God` cheat).
	 */
	float ApplyHit(const FCombatHit& Hit);
	/** R-SYN-13/14: pure armor math; a hit's newly applied states affect subsequent hits only. */
	static float ComputeDamageAfterArmor(float Damage, float Armor, bool bArmorBroken, float ArmorBrokenMultiplier);
	float Heal(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FHealthHealedSignature OnHealed;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetBaseArmor() const { return BaseArmor; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const { return !bDead; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FHealthDamagedSignature OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FHealthDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

	/** Fallback when the owner has no definition; owners normally call InitializeHealth. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;

	/** Fraction of damage blocked, 0-0.9. Set by the owner from its definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0", ClampMax = "0.9"))
	float BaseArmor = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	float CurrentHealth = 0.f;

private:
	bool bInitialized = false;
	bool bDead = false;
};
