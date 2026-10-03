#include "Combat/CombatStateComponent.h"

UCombatStateComponent::UCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCombatStateComponent::ApplyPoiseDamage(float Amount, AActor* Instigator)
{
	return false;
}

void UCombatStateComponent::ApplyState(FGameplayTag State, float Duration, AActor* Instigator)
{
	if (!State.IsValid() || ActiveStates.HasTagExact(State))
	{
		return;
	}
	ActiveStates.AddTag(State);
	OnStateAdded.Broadcast(State, Instigator);
}

void UCombatStateComponent::RemoveState(FGameplayTag State)
{
	if (ActiveStates.RemoveTag(State))
	{
		OnStateRemoved.Broadcast(State);
	}
}
