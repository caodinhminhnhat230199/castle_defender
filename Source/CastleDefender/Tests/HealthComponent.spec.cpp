#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/HealthComponent.h"
#include "Tests/CombatTestListener.h"

BEGIN_DEFINE_SPEC(FHealthComponentSpec, "CastleDefender.Combat.Health", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UHealthComponent* Health = nullptr;
	UCombatTestListener* Listener = nullptr;
END_DEFINE_SPEC(FHealthComponentSpec)

void FHealthComponentSpec::Define()
{
	BeforeEach([this]()
	{
		Health = NewObject<UHealthComponent>();
		Health->InitializeHealth(100.f, 0.f);
		Listener = NewObject<UCombatTestListener>();
		Health->OnDeath.AddDynamic(Listener, &UCombatTestListener::HandleDeath);
	});

	It("fires OnDeath exactly once when hit for max health", [this]()
	{
		FCombatHit Hit;
		Hit.Damage = 100.f;

		TestEqual("Applied damage", Health->ApplyHit(Hit), 100.f);
		TestTrue("Dead", Health->IsDead());
		TestEqual("OnDeath count", Listener->DeathCount, 1);

		TestEqual("Second hit applies nothing", Health->ApplyHit(Hit), 0.f);
		TestEqual("OnDeath count after second hit", Listener->DeathCount, 1);
	});

	It("survives a partial hit", [this]()
	{
		FCombatHit Hit;
		Hit.Damage = 40.f;

		Health->ApplyHit(Hit);
		TestEqual("Current health", Health->GetCurrentHealth(), 60.f);
		TestFalse("Dead", Health->IsDead());
		TestEqual("OnDeath count", Listener->DeathCount, 0);
	});

	AfterEach([this]()
	{
		Health = nullptr;
		Listener = nullptr;
	});
}

#endif
