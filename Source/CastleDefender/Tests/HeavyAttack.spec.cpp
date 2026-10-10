#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Tests/CombatTestListener.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/TestDummy.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
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

	It("refuses Heavy at 20 stamina and emits the insufficient-stamina cue producer", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		auto* Stamina = Fixture.Hero->GetStaminaComponent();
		Stamina->InitializeFromConfig(Fixture.Hero->GetHeroClassDefinition()->Stamina);
		TestTrue("Drain to 20", Stamina->TrySpend(Stamina->GetCurrentStamina() - 20.f));
		UCombatTestListener* Listener = NewObject<UCombatTestListener>();
		Stamina->OnStaminaSpendFailed.AddDynamic(Listener, &UCombatTestListener::HandleStaminaSpendFailed);

		TestFalse("Heavy refused", Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Heavy));
		TestEqual("Stays Idle", Fixture.Hero->GetCombatComponent()->GetActionState(), EHeroActionState::Idle);
		TestEqual("Stamina unchanged", Stamina->GetCurrentStamina(), 20.f);
		TestEqual("Failure cue producer", Listener->StaminaSpendFailedCount, 1);
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

	It("breaks a MaxPoise 50 dummy with Heavy then Light hits as the data implies (AC-CMB-05)", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		const UHeroClassDefinition* Def = Fixture.Hero->GetHeroClassDefinition();
		ATestDummy* Dummy = Fixture.World->SpawnActor<ATestDummy>(FVector(150.f, 0.f, 0.f), FRotator::ZeroRotator);
		Dummy->DispatchBeginPlay();
		UCombatStateComponent* Poise = Dummy->GetCombatState();
		const float MaxPoise = Poise->GetMaxPoise();

		TestTrue("Heavy accepted", Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Heavy));
		const FCombatHit Heavy = Fixture.Hero->GetMeleeTraceComponent()->GetPendingAttackTemplate();
		UCombatLibrary::DeliverHit(Dummy, Heavy);
		TestEqual("Heavy poise damage", Poise->GetCurrentPoise(), MaxPoise - Def->Heavy.PoiseDamage);
		TestFalse("Heavy alone does not stagger", Poise->HasState(GameTags::State_Combat_Staggered));

		// Light payloads from the authored chain until poise breaks.
		const float LightPoise = Def->LightChain[0].PoiseDamage;
		const int32 Expected = FMath::CeilToInt((MaxPoise - Def->Heavy.PoiseDamage) / LightPoise);
		int32 Lights = 0;
		while (!Poise->HasState(GameTags::State_Combat_Staggered) && Lights < 10)
		{
			FCombatHit Light = Heavy;
			Light.bIsHeavy = false;
			Light.Damage = Def->LightChain[0].Damage;
			Light.PoiseDamage = LightPoise;
			UCombatLibrary::DeliverHit(Dummy, Light);
			++Lights;
		}
		TestTrue("Staggered", Poise->HasState(GameTags::State_Combat_Staggered));
		TestEqual("Light hits implied by data", Lights, Expected);
		TestEqual("Instigator is the hero", Poise->GetStateInstigator(GameTags::State_Combat_Staggered), static_cast<AActor*>(Fixture.Hero));
	});

	It("carries AppliedStates from a test copy of the DA onto the target (Armor Broken hook)", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		UHeroClassDefinition* Def = Fixture.Hero->GetHeroClassDefinition();
		TestTrue("P1 saved Heavy applies Armor Broken", Def->Heavy.AppliedStates.HasTagExact(GameTags::State_Combat_ArmorBroken));
		Def->Heavy.StateDuration = 6.f;
		ATestDummy* Dummy = Fixture.World->SpawnActor<ATestDummy>(FVector(150.f, 0.f, 0.f), FRotator::ZeroRotator);
		Dummy->DispatchBeginPlay();
		Dummy->GetHealth()->InitializeHealth(100.f, 0.5f);

		// Isolate hit-payload propagation from presentation; authored P1 content is checked separately.
		UGameTuningSettings* Settings = GetMutableDefault<UGameTuningSettings>();
		const TSoftObjectPtr<UDataTable> SavedTable = Settings->CombatStatePresentationTable;
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FCombatStatePresentationRow::StaticStruct();
		FCombatStatePresentationRow Row;
		Row.StateTag = GameTags::State_Combat_ArmorBroken;
		Table->AddRow(Row.StateTag.GetTagName(), Row);
		Settings->CombatStatePresentationTable = Table;

		TestTrue("Heavy accepted", Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Heavy));
		UCombatLibrary::DeliverHit(Dummy, Fixture.Hero->GetMeleeTraceComponent()->GetPendingAttackTemplate());
		TestTrue("Armor Broken applied", Dummy->GetCombatState()->HasState(GameTags::State_Combat_ArmorBroken));
		TestEqual("Hit duration", Dummy->GetCombatState()->GetStateRemaining(GameTags::State_Combat_ArmorBroken), 6.f, 0.001f);
		Settings->CombatStatePresentationTable = SavedTable;
	});
}
#endif
