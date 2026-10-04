#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Hero/HeroCombatTypes.h"
#include "HeroCombatLibrary.generated.h"

class UAnimMontage;

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

	/** Editor setup only: scales a single-segment timing fixture to its authored duration. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static bool SetSingleSegmentMontageDuration(UAnimMontage* Montage, float Duration);

	/** Clears all combat hit and cancel window notify states from a montage to allow idempotent authoring. */
	UFUNCTION(BlueprintCallable, Category = "Hero Combat|Montage")
	static void ClearCombatNotifiesFromMontage(UAnimMontage* Montage);
};
