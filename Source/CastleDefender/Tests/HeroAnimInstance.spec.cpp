#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Hero/HeroAnimInstance.h"
#include "Hero/HeroCombatComponent.h"

BEGIN_DEFINE_SPEC(FHeroAnimInstanceSpec, "CastleDefender.Combat.Hero.AnimInstance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroAnimInstanceSpec)

void FHeroAnimInstanceSpec::Define()
{
	It("fades foot IK out during a montage and back in after it ends", [this]()
	{
		FHeroCombatFixture Fixture;
		USkeletalMeshComponent* Mesh = Fixture.Hero->GetMesh();
		Mesh->SetAnimInstanceClass(UHeroAnimInstance::StaticClass());
		const UHeroAnimInstance* Anim = Cast<UHeroAnimInstance>(Mesh->GetAnimInstance());
		if (!TestNotNull("Hero anim instance", Anim)) { return; }
		TestEqual("Locomotion keeps foot IK", Anim->GetFootIKAlpha(), 1.f);

		TestTrue("Light accepted", Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Light));
		Mesh->TickAnimation(0.05f, false);
		TestTrue("Fading out", Anim->GetFootIKAlpha() < 1.f && Anim->GetFootIKAlpha() > 0.f);
		Mesh->TickAnimation(0.2f, false);
		TestEqual("Montage plays without foot IK", Anim->GetFootIKAlpha(), 0.f);

		Mesh->GetAnimInstance()->Montage_Stop(0.f);
		Mesh->TickAnimation(0.2f, false);
		TestEqual("Foot IK restored after the montage", Anim->GetFootIKAlpha(), 1.f);
	});
}
#endif
