#include "Army/SquadDefinition.h"
#include "Army/SoldierCharacter.h"
#include "Core/GameTags.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

bool USquadDefinition::ValidateDefinition(FString& OutError) const
{
	OutError.Reset();
	const auto Nonnegative = [](float Value) { return FMath::IsFinite(Value) && Value >= 0.f; };
	const auto Positive = [&Nonnegative](float Value) { return Nonnegative(Value) && Value > 0.f; };
	const auto Ratio = [&Nonnegative](float Value) { return Nonnegative(Value) && Value <= 1.f; };
	if (DisplayName.IsEmpty() || !SoldierClass || SoldierClass->HasAnyClassFlags(CLASS_Abstract)
		|| !(SquadTypeTag == GameTags::Unit_Squad_Infantry || SquadTypeTag == GameTags::Unit_Squad_Archer || SquadTypeTag == GameTags::Unit_Squad_Spearman)
		|| SoldierCount <= 0 || FormationColumns <= 0 || MinFormationColumns <= 0 || MinFormationColumns > FormationColumns
		|| !Positive(FormationSpacing) || !Positive(MoveSpeed) || !Positive(MaxHealth)
		|| !Nonnegative(BaseArmor) || BaseArmor > 0.9f || !Nonnegative(EngageRadius)
		|| !Nonnegative(GuardLeashRadius) || !Nonnegative(FollowLeashRadius) || !Nonnegative(AttackLeashRadius)
		|| EngageRadius > GuardLeashRadius || EngageRadius > FollowLeashRadius || EngageRadius > AttackLeashRadius
		|| FollowOffset.ContainsNaN() || MaxAttackersPerTarget <= 0 || TargetRules.IsEmpty()
		|| !Nonnegative(CombatState.MaxPoise) || !Nonnegative(CombatState.PoiseRegenDelay) || !Nonnegative(CombatState.PoiseRegenRate)
		|| !Positive(CombatState.StaggerDuration) || !Nonnegative(Damage) || !Nonnegative(PoiseDamage)
		|| !Positive(AttackInterval) || !Positive(AttackRange) || !Nonnegative(ReformTimeout) || !Ratio(ReformRatio)
		|| !Ratio(LowStrengthThreshold) || !Positive(ReplenishInterval) || !Nonnegative(WipeRespawnDelay))
	{
		OutError = TEXT("Squad requires a display name, native squad type, concrete soldier class, positive count/columns/stats, engage within leashes and finite tuning.");
		return false;
	}
	for (const FSquadTargetRule& Rule : TargetRules)
	{
		if (!Nonnegative(Rule.MaxRange)) { OutError = TEXT("Squad target-rule range must be finite and nonnegative."); return false; }
	}
	for (const auto& Weight : StateScoreWeights)
	{
		if (!Weight.Key.IsValid() || !FMath::IsFinite(Weight.Value)) { OutError = TEXT("Squad state weights require valid tags and finite values."); return false; }
	}
	return true;
}

#if WITH_EDITOR
EDataValidationResult USquadDefinition::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Result = Super::IsDataValid(Context);
	FString Error;
	if (!ValidateDefinition(Error)) { Context.AddError(FText::FromString(Error)); return EDataValidationResult::Invalid; }
	return Result;
}
#endif
