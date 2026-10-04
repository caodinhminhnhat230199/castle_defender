#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_CombatHitWindow.generated.h"

/**
 * Montage notify state that defines active melee trace hit windows (T-CMB-04).
 * Begins and ends hit tracking on the owner's UMeleeTraceComponent.
 */
UCLASS(meta = (DisplayName = "Combat Hit Window"))
class CASTLEDEFENDER_API UAnimNotifyState_CombatHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Combat Hit Window");
	}
};
