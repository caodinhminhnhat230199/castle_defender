#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Hero/HeroCombatTypes.h"
#include "CombatActionTiming.generated.h"

class UHeroCombatComponent;

/** Derived timing view from montage notify states for validation and debugging (technical-plan §3.2, tasks T-CMB-02). */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FCombatActionTiming
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	float TotalDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	float CancelWindowStart = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	float CancelWindowEnd = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	TArray<EHeroAction> AllowedCancelActions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	bool bHasCancelWindow = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	bool bHasInvulnerableWindow = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
	bool bHasParryWindow = false;

	/** Inspects a montage's authored notify states and builds a timing summary. */
	static bool InspectMontage(const UAnimMontage* Montage, FCombatActionTiming& OutTiming, FString* OutError = nullptr);
};

/**
 * Opens a cancel window allowing specified actions during an animation montage.
 * State stays on owner's UHeroCombatComponent (never on notify object).
 */
UCLASS(meta = (DisplayName = "Hero Cancel Window"))
class CASTLEDEFENDER_API UAnimNotifyState_CancelWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancel")
	TArray<EHeroAction> AllowedActions;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

/**
 * Grants invulnerability / i-frames (e.g. during Dodge).
 */
UCLASS(meta = (DisplayName = "Hero Invulnerable Window"))
class CASTLEDEFENDER_API UAnimNotifyState_Invulnerable : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

/**
 * Opens parry reaction window.
 */
UCLASS(meta = (DisplayName = "Hero Parry Window"))
class CASTLEDEFENDER_API UAnimNotifyState_ParryWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
