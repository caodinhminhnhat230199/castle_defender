#pragma once

#include "CoreMinimal.h"
#include "Core/GameDefinition.h"
#include "Combat/CombatStateTypes.h"
#include "Army/SquadTypes.h"
#include "SquadDefinition.generated.h"

class ASoldierCharacter;
class UTexture2D;

/** Immutable squad tuning. Command, formation and combat consumers arrive in later SQD tasks. */
UCLASS(BlueprintType)
class CASTLEDEFENDER_API USquadDefinition : public UGameDefinition
{
	GENERATED_BODY()
public:
	bool ValidateDefinition(FString& OutError) const;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad", meta = (Categories = "Unit.Squad"))
	FGameplayTag SquadTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	TObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad")
	TSubclassOf<ASoldierCharacter> SoldierClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad", meta = (ClampMin = "1"))
	int32 SoldierCount = 8;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1"))
	int32 FormationColumns = 4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1"))
	int32 MinFormationColumns = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "1", Units = "cm"))
	float FormationSpacing = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "1", Units = "cm/s"))
	float MoveSpeed = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0", Units = "cm"))
	float EngageRadius = 800.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0", Units = "cm"))
	float GuardLeashRadius = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0", Units = "cm"))
	float FollowLeashRadius = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0", Units = "cm"))
	float AttackLeashRadius = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector FollowOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	TArray<FSquadTargetRule> TargetRules = { FSquadTargetRule() };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "1"))
	int32 MaxAttackersPerTarget = 2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "0", ClampMax = "0.9"))
	float BaseArmor = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier")
	FCombatStateConfig CombatState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "0"))
	float Damage = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "0"))
	float PoiseDamage = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "0.01", Units = "s"))
	float AttackInterval = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier", meta = (ClampMin = "1", Units = "cm"))
	float AttackRange = 150.f;
	/** Actor-typed content reference until T-SQD-10 introduces ACombatProjectile; no parallel projectile base. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soldier")
	TSubclassOf<AActor> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (Categories = "State.Combat"))
	TMap<FGameplayTag, float> StateScoreWeights;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "0", Units = "s"))
	float ReformTimeout = 4.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Formation", meta = (ClampMin = "0", ClampMax = "1"))
	float ReformRatio = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strength", meta = (ClampMin = "0", ClampMax = "1"))
	float LowStrengthThreshold = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strength", meta = (ClampMin = "0.01", Units = "s"))
	float ReplenishInterval = 4.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Strength", meta = (ClampMin = "0", Units = "s"))
	float WipeRespawnDelay = 20.f;
};
