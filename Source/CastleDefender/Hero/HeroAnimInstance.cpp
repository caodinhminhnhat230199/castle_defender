#include "Hero/HeroAnimInstance.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/LockOnComponent.h"

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
	bIsLockedOn = Hero && Hero->GetLockOnComponent()->GetLockOnTarget();
	const FVector LocalVelocity = Hero ? Hero->GetActorRotation().UnrotateVector(Hero->GetVelocity()) : FVector::ZeroVector;
	StrafeForwardSpeed = LocalVelocity.X;
	StrafeRightSpeed = LocalVelocity.Y;
	bIsBlocking = Hero && Hero->GetCombatComponent() && Hero->GetCombatComponent()->GetActionState() == EHeroActionState::Block;
	const float GuardTarget = bIsBlocking ? 1.f : 0.f;
	GuardAlpha = GuardBlendTime > 0.f
		? FMath::FInterpConstantTo(GuardAlpha, GuardTarget, DeltaSeconds, 1.f / GuardBlendTime)
		: GuardTarget;
}
