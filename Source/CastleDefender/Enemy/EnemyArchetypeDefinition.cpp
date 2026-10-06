#include "Enemy/EnemyArchetypeDefinition.h"
#include "Enemy/EnemyCharacter.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Combat/CombatActionTiming.h"
#include "Core/GameTuningSettings.h"
#endif

bool UEnemyArchetypeDefinition::ValidateDefinition(FString& OutError) const
{
	OutError.Reset();
	const auto Nonnegative = [](float Value) { return FMath::IsFinite(Value) && Value >= 0.f; };
	if (!EnemyClass || !Nonnegative(MaxHealth) || MaxHealth <= 0.f || !Nonnegative(BaseArmor) || BaseArmor > 0.9f
		|| !Nonnegative(WalkSpeed) || WalkSpeed <= 0.f || !Nonnegative(DecisionInterval) || DecisionInterval <= 0.f
		|| !Nonnegative(LocalAggroRadius) || !Nonnegative(MinTimeBetweenAttacks) || !Nonnegative(WindUpTurnRate)
		|| !Nonnegative(DespawnDelay) || !Nonnegative(CombatState.MaxPoise) || CombatState.MaxPoise <= 0.f
		|| !Nonnegative(CombatState.PoiseRegenDelay) || !Nonnegative(CombatState.PoiseRegenRate)
		|| !Nonnegative(CombatState.StaggerDuration) || CombatState.StaggerDuration <= 0.f)
	{
		OutError = TEXT("Enemy requires a class, positive health/speed/decision interval/poise/stagger duration and finite nonnegative tuning (armor <= 0.9).");
		return false;
	}
	if (Attacks.IsEmpty()) { OutError = TEXT("Enemy requires at least one attack."); return false; }
	if (TargetPriority.IsEmpty()) { OutError = TEXT("Enemy requires at least one target kind in TargetPriority."); return false; }
	for (const FEnemyAttackDefinition& Attack : Attacks)
	{
		if (!Attack.Montage || !Nonnegative(Attack.Range) || Attack.Range <= 0.f
			|| !Nonnegative(Attack.Damage) || Attack.Damage <= 0.f || !Nonnegative(Attack.PoiseDamage)
			|| !Nonnegative(Attack.PlayRate) || Attack.PlayRate <= 0.f || !Nonnegative(Attack.Cooldown)
			|| !Nonnegative(Attack.Weight) || Attack.Weight <= 0.f)
		{
			OutError = TEXT("Enemy attacks require a montage, positive range/damage/play rate/weight and finite nonnegative poise/cooldown.");
			return false;
		}
	}
	return true;
}

FEnemyRuntimeParams UEnemyArchetypeDefinition::MakeRuntimeParams() const
{
	FEnemyRuntimeParams Params;
	Params.UnitTags = UnitTags;
	Params.MaxHealth = MaxHealth;
	Params.BaseArmor = BaseArmor;
	Params.CombatState = CombatState;
	Params.WalkSpeed = WalkSpeed;
	Params.Attacks = Attacks;
	Params.DecisionInterval = DecisionInterval;
	Params.LocalAggroRadius = LocalAggroRadius;
	Params.TargetPriority = TargetPriority;
	Params.MinTimeBetweenAttacks = MinTimeBetweenAttacks;
	Params.WindUpTurnRate = WindUpTurnRate;
	Params.DespawnDelay = DespawnDelay;
	return Params;
}

#if WITH_EDITOR
EDataValidationResult UEnemyArchetypeDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	FString Error;
	if (!ValidateDefinition(Error))
	{
		Context.AddError(FText::FromString(Error));
		return EDataValidationResult::Invalid;
	}
	// R-ENM-05 / AC-ENM-02: warn when an attack's wind-up (montage start → first hit window) is below the global minimum.
	const float MinTelegraph = UGameTuningSettings::Get()->MinEnemyTelegraphTime;
	for (const FEnemyAttackDefinition& Attack : Attacks)
	{
		FCombatActionTiming Timing;
		FString TimingError;
		if (!FCombatActionTiming::InspectMontage(Attack.Montage, Timing, &TimingError) || !Timing.bHasHitWindow)
		{
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("%s has no valid Combat Hit Window: %s"), *GetNameSafe(Attack.Montage), *TimingError)));
		}
		else if (Timing.HitWindowStart / Attack.PlayRate < MinTelegraph)
		{
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("%s wind-up %.2f s is below the minimum telegraph time %.2f s."),
				*GetNameSafe(Attack.Montage), Timing.HitWindowStart / Attack.PlayRate, MinTelegraph)));
		}
	}
	return Result;
}
#endif
