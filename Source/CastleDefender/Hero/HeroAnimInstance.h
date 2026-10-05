#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeroAnimInstance.generated.h"

/**
 * Native parent of the hero Anim Blueprint. Exposes the foot IK weight so montages
 * (attacks, dodges, reactions) keep their authored leg motion instead of being pinned to the ground.
 */
UCLASS()
class CASTLEDEFENDER_API UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	float GetFootIKAlpha() const { return FootIKAlpha; }

protected:
	/** Weight for the foot IK pass: 1 in locomotion, 0 while a montage is active. Drives the ABP Control Rig Alpha. */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float FootIKAlpha = 1.f;

	/** Seconds to fade foot IK out when a montage starts and back in after it ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK", meta = (ClampMin = "0.0"))
	float FootIKBlendTime = 0.15f;
};
