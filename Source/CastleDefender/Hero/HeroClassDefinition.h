#pragma once

#include "CoreMinimal.h"
#include "Core/GameDefinition.h"
#include "Hero/HeroCombatTypes.h"
#include "HeroClassDefinition.generated.h"

/**
 * Data definition for a playable Hero class (spec §4.4, foundation §9).
 * Tunables for health, locomotion, sprint, camera rig.
 */
UCLASS(BlueprintType)
class CASTLEDEFENDER_API UHeroClassDefinition : public UGameDefinition
{
	GENERATED_BODY()

public:
	/** Same action/data validation in editor and packaged runtime, before spending stamina. */
	bool ValidateLightAttack(int32 ChainIndex, FString& OutError) const;
	bool ValidateHeavyAttack(FString& OutError) const;
	bool ValidateDodge(EHeroDodgeDirection Direction, FString& OutError) const;
	bool ValidateHitReaction(bool bFromFront, FString& OutError) const;
	UHeroClassDefinition();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FHeroMovementData Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FHeroCameraData Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FHeroInputData Input;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	FStaminaConfig Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FHeroAttackData> LightChain;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FHeroAttackData Heavy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FHeroAttackAssistData AttackAssist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FHeroDodgeData Dodge;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FHeroHitReactData HitReact;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Dodge.StaminaCost."))
	float DodgeStaminaCost = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float HeavyStaminaCost = 25.f;
};
