#include "Hero/HeroAnimInstance.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"

void UHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Per-frame: the anim update already runs every frame, and the IK weight must fade with the pose.
	// Active excludes montages blending out, so IK returns while the pose blends back to locomotion.
	const float Target = GetCurrentActiveMontage() ? 0.f : 1.f;
	FootIKAlpha = FootIKBlendTime > 0.f
		? FMath::FInterpConstantTo(FootIKAlpha, Target, DeltaSeconds, 1.f / FootIKBlendTime)
		: Target;

	const AHeroCharacter* Hero = Cast<AHeroCharacter>(TryGetPawnOwner());
	bIsBlocking = Hero && Hero->GetCombatComponent() && Hero->GetCombatComponent()->GetActionState() == EHeroActionState::Block;
	const float GuardTarget = bIsBlocking ? 1.f : 0.f;
	GuardAlpha = GuardBlendTime > 0.f
		? FMath::FInterpConstantTo(GuardAlpha, GuardTarget, DeltaSeconds, 1.f / GuardBlendTime)
		: GuardTarget;
}
