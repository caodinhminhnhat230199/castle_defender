#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HeroAnimInstance.generated.h"

/**
 * Native parent of the hero Anim Blueprint. Exposes the foot IK weight so montages
 * (attacks, dodges, reactions) keep their authored leg motion instead of being pinned to the ground,
 * and the guard flag that drives the upper-body block pose.
 */
UCLASS()
class CASTLEDEFENDER_API UHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	float GetFootIKAlpha() const { return FootIKAlpha; }
	bool IsBlocking() const { return bIsBlocking; }
	float GetGuardAlpha() const { return GuardAlpha; }

protected:
	/** Read-only locomotion inputs for the authored strafe blendspace. */
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsLockedOn = false;
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float StrafeForwardSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float StrafeRightSpeed = 0.f;
	/** Weight for the foot IK pass: 1 in locomotion, 0 while a montage is active. Drives the ABP Control Rig Alpha. */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float FootIKAlpha = 1.f;

	/** Seconds to fade foot IK out when a montage starts and back in after it ends. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK", meta = (ClampMin = "0.0"))
	float FootIKBlendTime = 0.15f;

	/** True while the hero's combat component is in Block. Read-only mirror; the component owns the state. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsBlocking = false;

	/** Upper-body guard pose weight, faded toward bIsBlocking. Drives the ABP guard layer. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float GuardAlpha = 0.f;

	/** Seconds to raise or drop the guard pose. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float GuardBlendTime = 0.1f;
};
