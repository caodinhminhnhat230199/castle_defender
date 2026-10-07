#include "Hero/HeroClassDefinition.h"
#include "Combat/CombatActionTiming.h"

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

	LightChain.SetNum(3);
	LightChain[0].Damage = 10.f;
	LightChain[0].PoiseDamage = 5.f;
	LightChain[0].StaminaCost = 0.f;
	LightChain[0].TraceRadius = 25.f;

	LightChain[1].Damage = 10.f;
	LightChain[1].PoiseDamage = 5.f;
	LightChain[1].StaminaCost = 0.f;
	LightChain[1].TraceRadius = 25.f;

	LightChain[2].Damage = 14.f;
	LightChain[2].PoiseDamage = 10.f;
	LightChain[2].StaminaCost = 0.f;
	LightChain[2].TraceRadius = 25.f;

	Heavy.Damage = 30.f;
	Heavy.PoiseDamage = 40.f;
	Heavy.StaminaCost = 25.f;
	Heavy.TraceRadius = 30.f;
}

bool UHeroClassDefinition::ValidateLightAttack(int32 ChainIndex, FString& OutError) const
{
	OutError.Reset();
	if (!AttackAssist.IsValid()) { OutError = TEXT("AttackAssist bounds must be finite and nonnegative (angle <= 180)."); return false; }
	if (LightChain.Num() != 3 || !LightChain.IsValidIndex(ChainIndex))
	{
		OutError = TEXT("LightChain must have exactly three entries and a valid index.");
		return false;
	}
	const FHeroAttackData& Attack = LightChain[ChainIndex];
	FCombatActionTiming Timing;
	if (!FCombatActionTiming::InspectMontage(Attack.Montage, Timing, &OutError))
	{
		OutError = FString::Printf(TEXT("LightChain[%d] montage: %s"), ChainIndex, *OutError);
		return false;
	}
	if (!FMath::IsFinite(Attack.Damage) || Attack.Damage <= 0.f
		|| !FMath::IsFinite(Attack.PoiseDamage) || Attack.PoiseDamage < 0.f
		|| !FMath::IsFinite(Attack.TraceRadius) || Attack.TraceRadius <= 0.f
		|| !FMath::IsFinite(Attack.StaminaCost) || Attack.StaminaCost < 0.f
		|| !FMath::IsFinite(Attack.StateDuration) || Attack.StateDuration < 0.f
		|| !FMath::IsFinite(Attack.InterruptResistance) || Attack.InterruptResistance < 0.f
		|| Heavy.Damage <= Attack.Damage || Heavy.PoiseDamage <= Attack.PoiseDamage
		|| Attack.StaminaCost >= HeavyStaminaCost)
	{
		OutError = FString::Printf(TEXT("LightChain[%d]: invalid attack values or Light/Heavy ordering."), ChainIndex);
		return false;
	}
	const TArray<EHeroAction>& Allowed = Timing.AllowedCancelActions;
	const bool bChainStep = ChainIndex < 2;
	if (Timing.HitWindowCount != 1 || !Timing.bHasCancelWindow
		|| !Allowed.Contains(EHeroAction::Dodge) || !Allowed.Contains(EHeroAction::BlockStart)
		|| Allowed.Contains(EHeroAction::Light) != bChainStep
		|| Allowed.Contains(EHeroAction::Heavy) != bChainStep
		|| Allowed.Contains(EHeroAction::Parry) || Allowed.Contains(EHeroAction::Interact)
		|| Allowed.Contains(EHeroAction::BlockEnd))
	{
		OutError = FString::Printf(TEXT("LightChain[%d]: requires one hit window and the authored Light/Heavy/Dodge/Block cancel set (Dodge/Block only for hit 3)."), ChainIndex);
		return false;
	}
	return true;
}

bool UHeroClassDefinition::ValidateHeavyAttack(FString& OutError) const
{
	OutError.Reset();
	if (!AttackAssist.IsValid()) { OutError = TEXT("AttackAssist bounds must be finite and nonnegative (angle <= 180)."); return false; }
	FCombatActionTiming Timing;
	if (!FCombatActionTiming::InspectMontage(Heavy.Montage, Timing, &OutError))
	{
		OutError = FString::Printf(TEXT("Heavy montage: %s"), *OutError);
		return false;
	}
	if (!FMath::IsFinite(Heavy.Damage) || Heavy.Damage <= 0.f
		|| !FMath::IsFinite(Heavy.PoiseDamage) || Heavy.PoiseDamage < 0.f
		|| !FMath::IsFinite(Heavy.TraceRadius) || Heavy.TraceRadius <= 0.f
		|| !FMath::IsFinite(Heavy.StateDuration) || Heavy.StateDuration < 0.f
		|| !FMath::IsFinite(HeavyStaminaCost) || HeavyStaminaCost < 0.f)
	{
		OutError = TEXT("Heavy: invalid damage, poise, trace radius, state duration or stamina cost.");
		return false;
	}
	for (int32 Index = 0; Index < LightChain.Num(); ++Index)
	{
		if (Heavy.Damage <= LightChain[Index].Damage || Heavy.PoiseDamage <= LightChain[Index].PoiseDamage
			|| LightChain[Index].StaminaCost >= HeavyStaminaCost)
		{
			OutError = FString::Printf(TEXT("Heavy must out-damage, out-poise and out-cost LightChain[%d]."), Index);
			return false;
		}
	}
	// R-CMB-15: readable startup before the hit; recovery only cancels late, into Dodge.
	if (Timing.HitWindowCount != 1 || Timing.HitWindowStart <= 0.f || !Timing.bHasCancelWindow
		|| Timing.CancelWindowStart + KINDA_SMALL_NUMBER < Timing.HitWindowEnd
		|| Timing.AllowedCancelActions.Num() != 1 || !Timing.AllowedCancelActions.Contains(EHeroAction::Dodge))
	{
		OutError = TEXT("Heavy requires a startup, one hit window and a later Dodge-only cancel window.");
		return false;
	}
	return true;
}

bool UHeroClassDefinition::ValidateDodge(EHeroDodgeDirection Direction, FString& OutError) const
{
	FCombatActionTiming Timing;
	UAnimMontage* Montage = Dodge.GetMontage(Direction);
	if (!FMath::IsFinite(Dodge.StaminaCost) || Dodge.StaminaCost < 0.f
		|| !FMath::IsFinite(Dodge.RootMotionScale) || Dodge.RootMotionScale <= 0.f)
	{
		OutError = TEXT("Dodge cost must be nonnegative and root-motion scale must be positive.");
		return false;
	}
	if (!FCombatActionTiming::InspectMontage(Montage, Timing, &OutError)) { return false; }
	if (Timing.InvulnerableWindowCount != 1 || Timing.bHasHitWindow || Timing.bHasParryWindow
		|| Timing.InvulnerableWindowStart <= 0.f || Timing.InvulnerableWindowEnd >= Timing.TotalDuration
		|| !Timing.bHasCancelWindow || Timing.CancelWindowStart + KINDA_SMALL_NUMBER < Timing.InvulnerableWindowEnd
		|| Timing.AllowedCancelActions.Num() != 3
		|| !Timing.AllowedCancelActions.Contains(EHeroAction::Light)
		|| !Timing.AllowedCancelActions.Contains(EHeroAction::Heavy)
		|| !Timing.AllowedCancelActions.Contains(EHeroAction::BlockStart))
	{
		OutError = TEXT("Dodge requires one bounded i-frame window followed by Light/Heavy/Block recovery cancels, and no attack/parry window.");
		return false;
	}
	return true;
}

bool UHeroClassDefinition::ValidateHitReaction(bool bFromFront, FString& OutError) const
{
	FCombatActionTiming Timing;
	if (!FCombatActionTiming::InspectMontage(bFromFront ? HitReact.FrontMontage : HitReact.BackMontage, Timing, &OutError)) { return false; }
	if (Timing.bHasHitWindow || Timing.bHasInvulnerableWindow || Timing.bHasParryWindow
		|| !Timing.bHasCancelWindow || Timing.CancelWindowStart <= 0.f
		|| Timing.AllowedCancelActions.Num() != 1 || !Timing.AllowedCancelActions.Contains(EHeroAction::Dodge))
	{
		OutError = TEXT("Hit reaction requires a late Dodge-only cancel window and no offensive/defensive window.");
		return false;
	}
	return true;
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

	if (!FMath::IsFinite(Heavy.InterruptResistance) || Heavy.InterruptResistance < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidHeavyResistance", "Heavy.InterruptResistance must be finite and non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	if (HeavyStaminaCost < 0.f)
	{
		Context.AddError(LOCTEXT("InvalidHeavyStaminaCost", "HeavyStaminaCost must be non-negative."));
		Result = EDataValidationResult::Invalid;
	}

	for (int32 Index = 0; Index < 3; ++Index)
	{
		FString Error;
		if (!ValidateLightAttack(Index, Error))
		{
			Context.AddError(FText::FromString(Error));
			Result = EDataValidationResult::Invalid;
		}
	}

	{
		FString Error;
		if (!ValidateHeavyAttack(Error))
		{
			Context.AddError(FText::FromString(Error));
			Result = EDataValidationResult::Invalid;
		}
	}

	for (EHeroDodgeDirection Direction : { EHeroDodgeDirection::Forward, EHeroDodgeDirection::Backward,
		EHeroDodgeDirection::Left, EHeroDodgeDirection::Right })
	{
		FString Error;
		if (!ValidateDodge(Direction, Error))
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Dodge[%d]: %s"), static_cast<int32>(Direction), *Error)));
			Result = EDataValidationResult::Invalid;
		}
	}
	for (bool bFront : { false, true })
	{
		FString Error;
		if (!ValidateHitReaction(bFront, Error))
		{
			Context.AddError(FText::FromString(Error));
			Result = EDataValidationResult::Invalid;
		}
	}
	if (!FMath::IsWithinInclusive(Block.DamageReduction, 0.f, 1.f) || Block.StaminaPerDamage < 0.f
		|| Block.ArcDegrees <= 0.f || Block.ArcDegrees > 360.f || Block.BlockRegenSuppressAfterHit < 0.f
		|| Block.BlockBreakStaggerDuration <= 0.f || Block.MoveSpeedMultiplier <= 0.f || Block.MoveSpeedMultiplier > 1.f)
	{
		Context.AddError(LOCTEXT("InvalidBlock", "Block: DamageReduction 0-1, StaminaPerDamage >= 0, ArcDegrees 0-360, suppression >= 0, BlockBreakStaggerDuration > 0, MoveSpeedMultiplier 0-1."));
		Result = EDataValidationResult::Invalid;
	}
	for (const UAnimMontage* Montage : { Block.BlockHitMontage.Get(), Block.BlockBreakMontage.Get() })
	{
		FCombatActionTiming Timing;
		FString Error;
		if (!FCombatActionTiming::InspectMontage(Montage, Timing, &Error)
			|| Timing.bHasHitWindow || Timing.bHasInvulnerableWindow || Timing.bHasParryWindow || Timing.bHasCancelWindow)
		{
			Context.AddError(LOCTEXT("InvalidBlockMontage", "Block: BlockHitMontage and BlockBreakMontage require valid montages with no combat windows."));
			Result = EDataValidationResult::Invalid;
		}
	}
	FCombatActionTiming DeathTiming;
	FString DeathError;
	if (!FCombatActionTiming::InspectMontage(HitReact.DeathMontage, DeathTiming, &DeathError)
		|| DeathTiming.bHasHitWindow || DeathTiming.bHasInvulnerableWindow || DeathTiming.bHasParryWindow || DeathTiming.bHasCancelWindow)
	{
		Context.AddError(LOCTEXT("InvalidDeathMontage", "Death requires a valid montage with no combat windows."));
		Result = EDataValidationResult::Invalid;
	}
	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif

#undef LOCTEXT_NAMESPACE
