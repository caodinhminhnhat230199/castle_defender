#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatStateTypes.generated.h"

/**
 * Per-unit poise tuning (R-SYN-01, R-SYN-05). ENM/SQD/BOS definitions embed this struct and the owner
 * passes it to UCombatStateComponent::Init at BeginPlay. The hero uses MaxPoise 0 (NEW-SYN-08).
 */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FCombatStateConfig
{
	GENERATED_BODY()

	/** [TUNABLE] 0 = never poise-staggered. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise", meta = (ClampMin = "0"))
	float MaxPoise = 0.f;

	/** [TUNABLE] Seconds after the last poise damage before regen starts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise", meta = (ClampMin = "0", Units = "s"))
	float PoiseRegenDelay = 2.f;

	/** [TUNABLE] Poise per second once regen starts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise", meta = (ClampMin = "0"))
	float PoiseRegenRate = 25.f;

	/** [TUNABLE] Staggered duration on poise break. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise", meta = (ClampMin = "0", Units = "s"))
	float StaggerDuration = 1.5f;
};

/** One active State.Combat.* entry. Expiry is in game seconds (R-SYN-08). */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FActiveCombatState
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag StateTag;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	double ExpiryTime = 0.0;

	/** Latest instigator; may be null once that actor is destroyed. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TWeakObjectPtr<AActor> Instigator;
};

/** DT_CombatStatePresentation row (row name = StateTag). P0 columns only; T-SYN-04 adds icon/colour/priority/VFX. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FCombatStatePresentationRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (Categories = "State.Combat"))
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Feedback"))
	FGameplayTag AppliedFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Feedback"))
	FGameplayTag RemovedFeedback;
};
