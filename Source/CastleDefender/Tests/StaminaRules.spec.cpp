#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Hero/HeroCombatTypes.h"
#include "Hero/StaminaComponent.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"
#include "Tests/CombatTestListener.h"

BEGIN_DEFINE_SPEC(FStaminaRulesSpec, "CastleDefender.Combat.Hero.Stamina", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FStaminaRulesSpec)

void FStaminaRulesSpec::Define()
{
	Describe("Pure Stamina State (FStaminaState / AC-CMB-15)", [this]()
	{
		It("executes full AC-CMB-15 lifecycle test case", [this]()
		{
			FStaminaConfig Config;
			Config.Max = 100.f;
			Config.RegenDelay = 0.8f;
			Config.RegenRate = 30.f;
			Config.BlockingRegenMultiplier = 0.5f;
			Config.SprintDrainPerSecond = 0.f;

			FStaminaState State;
			State.Init(Config);

			TestEqual("Initial stamina is max", State.Current, 100.f);

			// spend 30 -> 70
			TestTrue("Spend 30 succeeded", State.TrySpend(30.f, 0.0));
			TestEqual("Current is 70", State.Current, 70.f);

			// spend 80 -> rejected, 70
			TestFalse("Spend 80 rejected", State.TrySpend(80.f, 0.0));
			TestEqual("Current remains 70", State.Current, 70.f);

			// advance to 0.5 s -> 70
			State.Advance(0.5f, 0.5, false, Config);
			TestEqual("Current after 0.5s is still 70 (delay active)", State.Current, 70.f);

			// advance to 1.8 s -> 100 (clamped)
			State.Advance(1.3f, 1.8, false, Config);
			TestEqual("Current after 1.8s is 100 clamped", State.Current, 100.f);

			// same with blocking -> 85
			State.Init(Config);
			TestTrue("Spend 30 for blocking test", State.TrySpend(30.f, 0.0));
			TestEqual("Current is 70", State.Current, 70.f);

			State.Advance(0.5f, 0.5, true, Config);
			TestEqual("Current after 0.5s blocking is 70", State.Current, 70.f);

			State.Advance(1.3f, 1.8, true, Config);
			TestEqual("Current after 1.8s blocking is 85", State.Current, 85.f);

			// ApplyDamage(200) -> 0 and returns depleted
			const bool bDepleted = State.ApplyDamage(200.f, 1.8);
			TestTrue("ApplyDamage 200 returned depleted", bDepleted);
			TestEqual("Current is 0", State.Current, 0.f);
		});

		It("suppresses blocking regeneration during BlockRegenSuppressAfterHit", [this]()
		{
			FStaminaConfig Config;
			Config.Max = 100.f;
			Config.RegenDelay = 0.8f;
			Config.RegenRate = 30.f;
			Config.BlockingRegenMultiplier = 0.5f;

			FStaminaState State;
			State.Init(Config);
			State.TrySpend(30.f, 0.0); // Current = 70, LastSpend = 0.0

			// Blocked hit at 0.5s suppresses regen for 1.0s -> BlockedRegenUntil = 1.5
			State.OnBlockedHit(0.5, 1.0);

			// Advance to 1.3s: delay (0.8s) passed, but blocked suppression (1.5s) still active
			State.Advance(1.3f, 1.3, true, Config);
			TestEqual("Regen suppressed while BlockedRegenUntil is active", State.Current, 70.f);

			// Advance from 1.3s to 1.8s: active for 0.3s (1.5s to 1.8s)
			State.Advance(0.5f, 1.8, true, Config);
			// 70 + 30 * 0.5 * 0.3 = 74.5
			TestEqual("Regen resumes after suppression expires", State.Current, 74.5f);
		});

		It("drains stamina during sprint", [this]()
		{
			FStaminaConfig Config;
			Config.Max = 100.f;
			Config.SprintDrainPerSecond = 20.f;

			FStaminaState State;
			State.Init(Config);

			State.DrainSprint(1.5f, 1.5, Config.SprintDrainPerSecond);
			TestEqual("Stamina drained 30 over 1.5s", State.Current, 70.f);
		});
	});

	Describe("UStaminaComponent Lifecycle and Integration", [this]()
	{
		It("disables tick when full and enables tick when below max", [this]()
		{
			UStaminaComponent* Comp = NewObject<UStaminaComponent>();
			FStaminaConfig Config;
			Config.Max = 100.f;
			Config.RegenDelay = 0.1f;
			Config.RegenRate = 100.f;
			Comp->InitializeFromConfig(Config);

			TestFalse("Tick disabled when stamina is full", Comp->IsComponentTickEnabled());

			const bool bSpent = Comp->TrySpend(40.f);
			TestTrue("Spend succeeded", bSpent);
			TestTrue("Tick enabled when stamina is below max", Comp->IsComponentTickEnabled());
			TestEqual("Current is 60", Comp->GetCurrentStamina(), 60.f);

			// Simulate component tick past delay to restore to full
			Comp->TickComponent(1.5f, LEVELTICK_All, nullptr);
			TestEqual("Stamina restored to 100", Comp->GetCurrentStamina(), 100.f);
			TestFalse("Tick disabled again once restored to full", Comp->IsComponentTickEnabled());
		});

		It("honors InfiniteStamina cheat mode", [this]()
		{
			UStaminaComponent* Comp = NewObject<UStaminaComponent>();
			FStaminaConfig Config;
			Config.Max = 100.f;
			Comp->InitializeFromConfig(Config);

			Comp->SetInfiniteStamina(true);
			TestTrue("HasInfiniteStamina", Comp->HasInfiniteStamina());
			TestFalse("Tick remains disabled", Comp->IsComponentTickEnabled());

			const bool bSpent = Comp->TrySpend(50.f);
			TestTrue("TrySpend returns true under InfiniteStamina", bSpent);
			TestEqual("Stamina remains full", Comp->GetCurrentStamina(), 100.f);

			const bool bDamaged = Comp->ApplyDamage(80.f);
			TestFalse("ApplyDamage returns false under InfiniteStamina", bDamaged);
			TestEqual("Stamina remains full after damage", Comp->GetCurrentStamina(), 100.f);
		});

		It("broadcasts delegates for spend, fail, and depletion", [this]()
		{
			UStaminaComponent* Comp = NewObject<UStaminaComponent>();
			FStaminaConfig Config;
			Config.Max = 100.f;
			Comp->InitializeFromConfig(Config);

			UCombatTestListener* Listener = NewObject<UCombatTestListener>();
			Comp->OnStaminaChanged.AddDynamic(Listener, &UCombatTestListener::HandleStaminaChanged);
			Comp->OnStaminaSpendFailed.AddDynamic(Listener, &UCombatTestListener::HandleStaminaSpendFailed);
			Comp->OnStaminaDepleted.AddDynamic(Listener, &UCombatTestListener::HandleStaminaDepleted);

			// Successful spend
			Comp->TrySpend(35.f);
			TestTrue("OnStaminaChanged fired on spend", Listener->StaminaChangedCount > 0);
			TestEqual("Changed value matches 65", Listener->LastStaminaCurrent, 65.f);

			// Failed spend
			Comp->TrySpend(80.f);
			TestEqual("OnStaminaSpendFailed fired", Listener->StaminaSpendFailedCount, 1);
			TestEqual("Failed cost recorded", Listener->LastFailedCost, 80.f);

			// Depletion via damage
			Comp->ApplyDamage(100.f);
			TestEqual("OnStaminaDepleted fired", Listener->StaminaDepletedCount, 1);
			TestTrue("IsDepleted", Comp->IsDepleted());
		});
	});
}

#endif
