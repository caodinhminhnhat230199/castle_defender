#include "Hero/HeroAnimInstance.h"

void UHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Per-frame: the anim update already runs every frame, and the IK weight must fade with the pose.
	// Active excludes montages blending out, so IK returns while the pose blends back to locomotion.
	const float Target = GetCurrentActiveMontage() ? 0.f : 1.f;
	FootIKAlpha = FootIKBlendTime > 0.f
		? FMath::FInterpConstantTo(FootIKAlpha, Target, DeltaSeconds, 1.f / FootIKBlendTime)
		: Target;
}
