#include "Hero/HeroClassDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "HeroClassDefinition"

UHeroClassDefinition::UHeroClassDefinition()
{
	DisplayName = LOCTEXT("WarlordDisplayName", "Warlord");
	MaxHealth = 200.f;
	Movement = FHeroMovementData();
	Camera = FHeroCameraData();
}

#if WITH_EDITOR
EDataValidationResult UHeroClassDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (MaxHealth <= 0.f)
	{
		Context.AddError(LOCTEXT("InvalidMaxHealth", "MaxHealth must be greater than 0."));
		Result = EDataValidationResult::Invalid;
	}

	if (Movement.JogSpeed <= 0.f)
	{
		Context.AddError(LOCTEXT("InvalidJogSpeed", "Movement.JogSpeed must be greater than 0."));
		Result = EDataValidationResult::Invalid;
	}

	if (Movement.SprintSpeed <= Movement.JogSpeed)
	{
		Context.AddError(LOCTEXT("InvalidSprintSpeed", "Movement.SprintSpeed must be greater than JogSpeed."));
		Result = EDataValidationResult::Invalid;
	}

	if (Movement.RotationRateYaw <= 0.f)
	{
		Context.AddError(LOCTEXT("InvalidRotationRate", "Movement.RotationRateYaw must be greater than 0."));
		Result = EDataValidationResult::Invalid;
	}

	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif

#undef LOCTEXT_NAMESPACE
