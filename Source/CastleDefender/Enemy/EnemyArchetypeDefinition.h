#pragma once

#include "CoreMinimal.h"
#include "Core/GameDefinition.h"
#include "Combat/CombatStateTypes.h"
#include "Animation/AnimMontage.h"
#include "EnemyArchetypeDefinition.generated.h"

class AEnemyCharacter;

/** R-ENM-02/05: montage authoring owns wind-up/hit/recovery; the definition owns attack tuning. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FEnemyAttackDefinition
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> Montage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Range = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Damage = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0"))
	float PoiseDamage = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	bool bIsHeavy = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.001"))
	float PlayRate = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Cooldown = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0"))
	float Weight = 1.f;
};

/** Runtime copy for spawn/phase overrides. No caller mutates a definition asset. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FEnemyRuntimeParams
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	FGameplayTagContainer UnitTags;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MaxHealth = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float BaseArmor = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	FCombatStateConfig CombatState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float WalkSpeed = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TArray<FEnemyAttackDefinition> Attacks;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float DecisionInterval = 0.2f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float LocalAggroRadius = 600.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float MinTimeBetweenAttacks = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float WindUpTurnRate = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	float DespawnDelay = 3.f;
};

UCLASS(BlueprintType)
class CASTLEDEFENDER_API UEnemyArchetypeDefinition : public UGameDefinition
{
	GENERATED_BODY()
public:
	/** Runtime safety validation shared with editor validation. */
	bool ValidateDefinition(FString& OutError) const;
	FEnemyRuntimeParams MakeRuntimeParams() const;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FGameplayTagContainer UnitTags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSubclassOf<AEnemyCharacter> EnemyClass;
	// Values with no approved spec default stay unset until content authoring/tuning (T-ENM-11).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "1"))
	float MaxHealth = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0", ClampMax = "0.9"))
	float BaseArmor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FCombatStateConfig CombatState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.001"))
	float WalkSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TArray<FEnemyAttackDefinition> Attacks;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.001"))
	float DecisionInterval = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float LocalAggroRadius = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float MinTimeBetweenAttacks = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float WindUpTurnRate = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0"))
	float DespawnDelay = 3.f;
};
