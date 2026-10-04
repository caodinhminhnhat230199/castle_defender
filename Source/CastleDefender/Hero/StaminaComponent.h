#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hero/HeroCombatTypes.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaSpendFailedSignature, float, AttemptedCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);

/**
 * UStaminaComponent manages hero stamina pool, expenditure, regeneration and sprint drain.
 * Operates on the hero's dilated clock domain (D-20).
 * Tick is enabled only while below max stamina or actively draining sprint.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initializes stamina config and resets state from data asset. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void InitializeFromConfig(const FStaminaConfig& InConfig);

	/** Attempts to spend stamina. Broadcasts OnStaminaChanged or OnStaminaSpendFailed. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool TrySpend(float Cost);

	/** Applies damage directly to stamina (e.g. from blocked attacks). Returns true if depleted. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	bool ApplyDamage(float Amount);

	/** Records a blocked hit to suppress regen for a duration. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void OnBlockedHit(float Suppression);

	/** Updates blocking state for regen multiplier. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetBlocking(bool bInBlocking);

	/** Enables or disables sprint drain. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetSprintDraining(bool bInDraining);

	/** Toggles infinite stamina cheat mode. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetInfiniteStamina(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetCurrentStamina() const { return State.Current; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return State.Max; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaFraction() const { return State.Max > 0.f ? State.Current / State.Max : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsDepleted() const { return State.Current <= 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool HasInfiniteStamina() const { return bInfiniteStamina; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsBlocking() const { return bBlocking; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsSprintDraining() const { return bSprintDraining; }

	const FStaminaConfig& GetConfig() const { return Config; }
	const FStaminaState& GetState() const { return State; }

	// Delegated events
	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaSpendFailedSignature OnStaminaSpendFailed;

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaDepletedSignature OnStaminaDepleted;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	FStaminaConfig Config;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	FStaminaState State;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	bool bBlocking = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	bool bSprintDraining = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	bool bInfiniteStamina = false;

	double HeroActionClock = 0.0;

	void UpdateTickState();
};
