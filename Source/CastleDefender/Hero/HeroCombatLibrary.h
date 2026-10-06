#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Hero/HeroCombatTypes.h"
#include "HeroCombatLibrary.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

/**
 * Blueprint and Python function library for Hero Combat setup and montage authoring (spec §4.4, technical-plan §3.5).
 */
UCLASS()
class CASTLEDEFENDER_API UHeroCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Adds an active melee combat hit window notify to a montage. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool AddCombatHitWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration);

	/** Adds an authored cancel window notify to a montage specifying which actions can interrupt it. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool AddCancelWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration, const TArray<EHeroAction>& AllowedActions);

	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool AddInvulnerableWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration);

	/** Adds a missing assist window without changing existing authored timing or adding duplicates. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool EnsureRotationAssistWindow(UAnimMontage* Montage);

	/** Editor setup only: scales a single-segment timing fixture to its authored duration. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool SetSingleSegmentMontageDuration(UAnimMontage* Montage, float Duration);

	/** Editor setup only: points a single-segment montage at Sequence (and its skeleton), trims it to
	 *  [AnimStartTime, AnimEndTime] and fits it to Duration, optionally reversed. Notifies are left untouched. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool SetSingleSegmentMontageSource(UAnimMontage* Montage, UAnimSequenceBase* Sequence, float AnimStartTime, float AnimEndTime, float Duration, bool bPlayReversed);

	/** Clears all combat hit and cancel window notify states from a montage to allow idempotent authoring. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static void ClearCombatNotifiesFromMontage(UAnimMontage* Montage);
};
