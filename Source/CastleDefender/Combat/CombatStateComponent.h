#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStateAddedSignature, FGameplayTag, State, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatStateRemovedSignature, FGameplayTag, State);

/**
 * Poise and timed combat states (State.Combat.*) for hero, soldiers, enemies and boss (D-05).
 * Skeleton: states are stored without timing. Poise, expiry timer and durations arrive in T-SYN-01.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatStateComponent();

	/** Returns true when poise breaks into State.Combat.Staggered. No poise model yet (T-SYN-01). */
	bool ApplyPoiseDamage(float Amount, AActor* Instigator);

	/** Adds the state. Duration is ignored until T-SYN-01 adds expiry. */
	void ApplyState(FGameplayTag State, float Duration, AActor* Instigator);

	void RemoveState(FGameplayTag State);

	/** True if the state or a child of it is active. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasState(FGameplayTag State) const { return ActiveStates.HasTag(State); }

	const FGameplayTagContainer& GetActiveStates() const { return ActiveStates; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FCombatStateAddedSignature OnStateAdded;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FCombatStateRemovedSignature OnStateRemoved;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	FGameplayTagContainer ActiveStates;
};
