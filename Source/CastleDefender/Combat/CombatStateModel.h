#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatStateTypes.h"

/**
 * Pure poise and timed-state math with an explicit Now (technical plan §4.3), so specs need no world.
 * Poise is lazy: stored value + regen computed on read, no Tick. States never stack (R-SYN-08).
 */
struct CASTLEDEFENDER_API FCombatStateModel
{
	enum class EApplyResult : uint8 { Added, Refreshed };

	/** Sets the config and fills poise; clears states. */
	void Init(const FCombatStateConfig& InConfig);

	const FCombatStateConfig& GetConfig() const { return Config; }

	/** 0 while Staggered or when the unit has no poise. */
	float GetPoise(double Now) const;

	/** Debug/cheat: sets current poise (clamped), restarting the regen delay. Ignored while Staggered. */
	void SetPoise(float Value, double Now);

	/** Returns true when this damage breaks poise; the caller then applies Staggered. Ignored while Staggered (R-SYN-04). */
	bool ApplyPoiseDamage(float Amount, double Now);

	/** Adds the state, or refreshes an active one to the later expiry and records the new instigator. */
	EApplyResult ApplyState(FGameplayTag State, float Duration, AActor* Instigator, double Now);

	/** True if the exact state was active. Removing Staggered refills poise (R-SYN-04). */
	bool RemoveState(FGameplayTag State);

	/** Removes every state with expiry <= Now (a late timer still clears all of them); returns their tags. */
	TArray<FGameplayTag> RemoveExpired(double Now);

	/** Removes every state and refills poise (R-SYN-10); returns the removed tags. */
	TArray<FGameplayTag> ClearAll();

	/** Earliest expiry, or unset when no state is active. */
	TOptional<double> NextExpiry() const;

	/** True if the state or a child of it is active. */
	bool HasState(FGameplayTag State) const;

	const FActiveCombatState* FindState(FGameplayTag State) const;

	const TArray<FActiveCombatState>& GetStates() const { return States; }

private:
	void RefillPoise();

	FCombatStateConfig Config;
	float StoredPoise = 0.f;
	double LastPoiseDamageTime = 0.0;
	TArray<FActiveCombatState> States;
};
