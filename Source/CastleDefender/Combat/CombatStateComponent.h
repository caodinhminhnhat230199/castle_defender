#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Combat/CombatStateModel.h"
#include "Combat/CombatTypes.h"
#include "CombatStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStateAddedSignature, FGameplayTag, State, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatStateRemovedSignature, FGameplayTag, State);

/**
 * Poise and timed combat states (State.Combat.*) for hero, soldiers, enemies and boss (D-05, R-SYN-08/09).
 * The owner calls Init with the FCombatStateConfig embedded in its ENM/SQD/BOS definition; the hero uses MaxPoise 0.
 * No Tick: poise is computed on read and one game-time timer targets the earliest expiry while any state is active.
 * Death of the sibling UHealthComponent clears every state (R-SYN-10).
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatStateComponent();

	/** Sets poise tuning and refills poise. Clears active states. */
	void Init(const FCombatStateConfig& Config);

	/** Returns true when poise breaks into State.Combat.Staggered (R-SYN-03). */
	bool ApplyPoiseDamage(float Amount, AActor* Instigator);

	/**
	 * Adds or refreshes a state (refresh keeps the longer expiry, no second OnStateAdded).
	 * Duration <= 0 uses Game Tuning StateDefaultDurations; if that is 0 too, the state is not applied.
	 * Ignored when the owner is dead.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyState(UPARAM(meta = (Categories = "State.Combat")) FGameplayTag State, float Duration, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RemoveState(FGameplayTag State);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearAllStates();

	/** True if the state or a child of it is active. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasState(FGameplayTag State) const { return Model.HasState(State); }

	/** Seconds left on the exact state, 0 when inactive. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetStateRemaining(FGameplayTag State) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentPoise() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxPoise() const { return Model.GetConfig().MaxPoise; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetStateInstigator(FGameplayTag State) const;

	/** True while the expiry timer is set (only while a state is active). */
	bool HasPendingExpiry() const;

	/** Debug/cheat entry (SetPoise). */
	void SetPoise(float Value);

	FGameplayTagContainer GetActiveStates() const;
	const TArray<FActiveCombatState>& GetStates() const { return Model.GetStates(); }

	/** New states only; refreshes do not fire. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FCombatStateAddedSignature OnStateAdded;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FCombatStateRemovedSignature OnStateRemoved;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	double Now() const;
	bool IsOwnerDead() const;
	void AddState(FGameplayTag State, float Duration, AActor* Instigator);
	void HandleRemoved(const TArray<FGameplayTag>& Removed);
	void RescheduleExpiry();
	void HandleExpiryTimer();
	void PlayPresentation(FGameplayTag State, bool bApplied, AActor* Instigator) const;

	UFUNCTION()
	void HandleOwnerDeath(const FCombatHit& KillingHit);

	FCombatStateModel Model;
	FTimerHandle ExpiryTimer;
};
