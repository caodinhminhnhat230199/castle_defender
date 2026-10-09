#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SquadTypes.generated.h"

class ASoldierCharacter;

UENUM(BlueprintType)
enum class ESquadState : uint8 { Idle, Follow, MoveToOrder, Guard, Engage, Reform, Retreat, Recover, Wiped };

UENUM(BlueprintType)
enum class ESquadTargetRuleType : uint8
{
	InGuardArea, FocusTarget, NearestWithTags, LowestHealthWithTags, NearestToGuardCenter, AttackingStructureNearGuard
};

USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FSquadTargetRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	ESquadTargetRuleType Type = ESquadTargetRuleType::NearestWithTags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	FGameplayTagContainer Tags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0", Units = "cm"))
	float MaxRange = 800.f;
};

USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FSquadOrder
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FGameplayTag CommandTag;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FRotator Facing = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	TWeakObjectPtr<AActor> TargetActor;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	TWeakObjectPtr<AActor> ContextActor;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	float LeashRadiusOverride = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FGuid OrderId;
};

USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FSoldierRuntime
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	TWeakObjectPtr<ASoldierCharacter> Soldier;
	UPROPERTY(BlueprintReadOnly, Category = "Squad")
	int32 SlotIndex = INDEX_NONE;
};
