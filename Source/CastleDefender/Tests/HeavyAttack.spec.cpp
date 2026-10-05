#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Combat/MeleeTraceComponent.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"

BEGIN_DEFINE_SPEC(FHeavyAttackSpec, "CastleDefender.Combat.Hero.Heavy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeavyAttackSpec)

void FHeavyAttackSpec::Define()
{
	It("refuses a Heavy without a valid montage, spends nothing and leaves Light usable", [this]()
	{
		FHeroCombatFixture Fixture;
		auto* Combat = Fixture.Hero->GetCombatComponent();
		auto* Stamina = Fixture.Hero->GetStaminaComponent();
		Stamina->InitializeFromConfig(Fixture.Hero->GetHeroClassDefinition()->Stamina);
		Fixture.Hero->GetHeroClassDefinition()->Heavy.Montage = nullptr;
		AddExpectedError(TEXT("Heavy attack refused"), EAutomationExpectedErrorFlags::Contains, 1);

		TestFalse("Heavy refused", Combat->RequestAction(EHeroAction::Heavy));
		TestEqual("Stays Idle", Combat->GetActionState(), EHeroActionState::Idle);
		TestEqual("No stamina spent", Stamina->GetCurrentStamina(), 100.f);
		TestTrue("Light still starts", Combat->RequestAction(EHeroAction::Light));
	});

	It("plays the Heavy montage with a heavy hit payload, spends stamina and returns to Idle", [this]()
	{
		FHeroCombatFixture Fixture;
		auto* Combat = Fixture.Hero->GetCombatComponent();
		auto* Stamina = Fixture.Hero->GetStaminaComponent();
		const UHeroClassDefinition* Def = Fixture.Hero->GetHeroClassDefinition();
		Stamina->InitializeFromConfig(Def->Stamina);

		TestTrue("Heavy accepted", Combat->RequestAction(EHeroAction::Heavy));
		TestEqual("HeavyAttack state", Combat->GetActionState(), EHeroActionState::HeavyAttack);
		TestEqual("Heavy montage playing", Fixture.Hero->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage(), Def->Heavy.Montage.Get());
		TestEqual("Stamina spent", Stamina->GetCurrentStamina(), 100.f - Def->HeavyStaminaCost);
		const FCombatHit& Pending = Fixture.Hero->GetMeleeTraceComponent()->GetPendingAttackTemplate();
		TestTrue("Hit is heavy", Pending.bIsHeavy);
		TestEqual("Heavy damage", Pending.Damage, Def->Heavy.Damage);
		TestEqual("Heavy poise damage", Pending.PoiseDamage, Def->Heavy.PoiseDamage);

		Fixture.Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
		Fixture.Hero->GetMesh()->TickAnimation(0.01f, false);
		Fixture.Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
		TestEqual("Montage end returns Idle", Combat->GetActionState(), EHeroActionState::Idle);
		TestTrue("Light starts after Heavy", Combat->RequestAction(EHeroAction::Light));
	});

	It("fails validation when Heavy is not stronger than every Light hit", [this]()
	{
		FHeroCombatFixture Fixture;
		UHeroClassDefinition* Def = Fixture.Hero->GetHeroClassDefinition();
		FString Error;
		TestTrue("Warlord Heavy valid", Def->ValidateHeavyAttack(Error));
		Def->Heavy.Damage = Def->LightChain[2].Damage;
		TestFalse("Heavy no stronger than Light 3", Def->ValidateHeavyAttack(Error));
	});
}
#endif
