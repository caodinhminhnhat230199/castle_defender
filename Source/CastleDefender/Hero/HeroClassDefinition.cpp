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
	Input = FHeroInputData();
	Stamina = FStaminaConfig();
	DodgeStaminaCost = 20.f;
	HeavyStaminaCost = 25.f;
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

	if (Input.InputBufferTime < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidInputBufferTime", "Input.InputBufferTime must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (Stamina.Max <= 0.f)
	{
		Context.AddError(LOCTEXT("InvalidMaxStamina", "Stamina.Max must be greater than 0."));
		Result = EDataValidationResult::Invalid;
	}

	if (Stamina.RegenDelay < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidRegenDelay", "Stamina.RegenDelay must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (Stamina.RegenRate < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidRegenRate", "Stamina.RegenRate must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (Stamina.BlockingRegenMultiplier < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidBlockingRegenMultiplier", "Stamina.BlockingRegenMultiplier must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (Stamina.SprintDrainPerSecond < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidSprintDrainPerSecond", "Stamina.SprintDrainPerSecond must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (DodgeStaminaCost < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidDodgeStaminaCost", "DodgeStaminaCost must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (HeavyStaminaCost < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidHeavyStaminaCost", "HeavyStaminaCost must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif

#undef LOCTEXT_NAMESPACE
