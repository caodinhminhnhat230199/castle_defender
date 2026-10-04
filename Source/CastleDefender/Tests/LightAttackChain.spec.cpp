#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Hero/HeroCombatTypes.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/StaminaComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/CombatTypes.h"
#include "Core/GameTags.h"
#include "Animation/AnimMontage.h"
#include "Misc/DataValidation.h"
#include "UObject/Package.h"
#include "Tests/HeroCombatFixture.h"

BEGIN_DEFINE_SPEC(FLightAttackChainSpec, "CastleDefender.Combat.Hero.LightChain", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FHeroCombatFixture* Fixture = nullptr;
	AHeroCharacter* Hero = nullptr;
	UHeroCombatComponent* CombatComp = nullptr;
	UStaminaComponent* StaminaComp = nullptr;
	UMeleeTraceComponent* TraceComp = nullptr;
	UHeroClassDefinition* ClassDef = nullptr;
END_DEFINE_SPEC(FLightAttackChainSpec)

void FLightAttackChainSpec::Define()
{
	BeforeEach([this]()
	{
		Fixture = new FHeroCombatFixture();
		Hero = Fixture->Hero;
		ClassDef = Hero->GetHeroClassDefinition();

		// Populate Warlord spec defaults for testing
		ClassDef->LightChain.SetNum(3);
		ClassDef->LightChain[0].Damage = 10.f;
		ClassDef->LightChain[0].PoiseDamage = 5.f;
		ClassDef->LightChain[0].StaminaCost = 0.f;
		ClassDef->LightChain[0].TraceRadius = 25.f;

		ClassDef->LightChain[1].Damage = 10.f;
		ClassDef->LightChain[1].PoiseDamage = 5.f;
		ClassDef->LightChain[1].StaminaCost = 0.f;
		ClassDef->LightChain[1].TraceRadius = 25.f;

		ClassDef->LightChain[2].Damage = 14.f;
		ClassDef->LightChain[2].PoiseDamage = 10.f;
		ClassDef->LightChain[2].StaminaCost = 0.f;
		ClassDef->LightChain[2].TraceRadius = 25.f;

		ClassDef->Heavy.Damage = 30.f;
		ClassDef->Heavy.PoiseDamage = 40.f;
		ClassDef->Heavy.StaminaCost = 25.f;
		ClassDef->Heavy.TraceRadius = 30.f;

		Hero->SetHeroClassDefinition(ClassDef);

		CombatComp = Hero->GetCombatComponent();
		StaminaComp = Hero->GetStaminaComponent();
		TraceComp = Hero->GetMeleeTraceComponent();

		if (StaminaComp)
		{
			StaminaComp->InitializeFromConfig(ClassDef->Stamina);
		}
	});

	AfterEach([this]()
	{
		delete Fixture;
		Fixture = nullptr;
		Hero = nullptr;
		ClassDef = nullptr;
		CombatComp = nullptr;
		StaminaComp = nullptr;
		TraceComp = nullptr;
	});

	Describe("Data Validation (IsDataValid)", [this]()
	{
		It("catches a missing third entry in LightChain (2 entries)", [this]()
		{
			ClassDef->LightChain.SetNum(2);
			FDataValidationContext Context;
			const EDataValidationResult ValidationResult = ClassDef->IsDataValid(Context);
			TestEqual("Validation fails for 2 entries", ValidationResult, EDataValidationResult::Invalid);
		});

		It("catches more than 3 entries in LightChain (4 entries)", [this]()
		{
			ClassDef->LightChain.SetNum(4);
			FDataValidationContext Context;
			const EDataValidationResult ValidationResult = ClassDef->IsDataValid(Context);
			TestEqual("Validation fails for 4 entries", ValidationResult, EDataValidationResult::Invalid);
		});

		It("catches null montage reference in LightChain entry", [this]()
		{
			ClassDef->LightChain[0].Montage = nullptr;
			FDataValidationContext Context;
			const EDataValidationResult ValidationResult = ClassDef->IsDataValid(Context);
			TestEqual("Validation fails for null montage", ValidationResult, EDataValidationResult::Invalid);
		});

		It("validates production DA_HeroClass_Warlord asset successfully", [this]()
		{
			UHeroClassDefinition* ProductionDA = LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord"));
			TestNotNull("DA_HeroClass_Warlord loaded", ProductionDA);
			if (ProductionDA)
			{
				FDataValidationContext Context;
				const EDataValidationResult ValidationResult = ProductionDA->IsDataValid(Context);
				TestEqual("Production asset passes IsDataValid", ValidationResult, EDataValidationResult::Valid);
				TestEqual("LightChain has exactly 3 entries", ProductionDA->LightChain.Num(), 3);
				TestEqual("Hit 1 damage is 10", ProductionDA->LightChain[0].Damage, 10.f);
				TestEqual("Hit 2 damage is 10", ProductionDA->LightChain[1].Damage, 10.f);
				TestEqual("Hit 3 damage is 14", ProductionDA->LightChain[2].Damage, 14.f);
			}
		});
	});

	Describe("Chain Progression and Reset Rules (AC-CMB-02)", [this]()
	{
		It("starts at chain index 0 from Idle", [this]()
		{
			TestEqual("Initial state is Idle", CombatComp->GetActionState(), EHeroActionState::Idle);
			TestEqual("Initial chain index is 0", CombatComp->GetCurrentChainIndex(), 0);

			const bool bAccepted = CombatComp->RequestAction(EHeroAction::Light);
			TestTrue("Light action accepted", bAccepted);
			TestEqual("State transitioned to LightAttack", CombatComp->GetActionState(), EHeroActionState::LightAttack);
			TestEqual("Chain index is 0", CombatComp->GetCurrentChainIndex(), 0);
		});

		It("advances chain index 0 -> 1 -> 2 when Light is requested in open chain window", [this]()
		{
			// Hit 1 (index 0)
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Hit 1 index is 0", CombatComp->GetCurrentChainIndex(), 0);

			// Open cancel/chain window allowing Light
			CombatComp->OpenCancelWindow({ EHeroAction::Light, EHeroAction::Heavy, EHeroAction::Dodge, EHeroAction::BlockStart });
			TestTrue("Cancel window allows Light", CombatComp->CanStartAction(EHeroAction::Light));

			// Hit 2 (index 1)
			const bool bHit2Accepted = CombatComp->RequestAction(EHeroAction::Light);
			TestTrue("Hit 2 accepted", bHit2Accepted);
			TestEqual("Hit 2 index is 1", CombatComp->GetCurrentChainIndex(), 1);
			TestEqual("State is LightAttack", CombatComp->GetActionState(), EHeroActionState::LightAttack);

			// Open cancel/chain window for Hit 2
			CombatComp->OpenCancelWindow({ EHeroAction::Light, EHeroAction::Heavy, EHeroAction::Dodge, EHeroAction::BlockStart });

			// Hit 3 (index 2)
			const bool bHit3Accepted = CombatComp->RequestAction(EHeroAction::Light);
			TestTrue("Hit 3 accepted", bHit3Accepted);
			TestEqual("Hit 3 index is 2", CombatComp->GetCurrentChainIndex(), 2);
			TestEqual("State is LightAttack", CombatComp->GetActionState(), EHeroActionState::LightAttack);
		});

		It("does not advance past hit 3 when cancel window excludes Light", [this]()
		{
			// Hit 1
			CombatComp->RequestAction(EHeroAction::Light);
			CombatComp->OpenCancelWindow({ EHeroAction::Light });

			// Hit 2
			CombatComp->RequestAction(EHeroAction::Light);
			CombatComp->OpenCancelWindow({ EHeroAction::Light });

			// Hit 3 (index 2)
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Hit 3 index is 2", CombatComp->GetCurrentChainIndex(), 2);

			// Hit 3 recovery cancel window allows only Dodge and Block (technical-plan §5.1)
			CombatComp->OpenCancelWindow({ EHeroAction::Dodge, EHeroAction::BlockStart });
			TestFalse("Hit 3 window rejects Light", CombatComp->CanStartAction(EHeroAction::Light));

			// 4th press during Hit 3 recovery is rejected and buffered, does not advance to index 3
			const bool b4thAccepted = CombatComp->RequestAction(EHeroAction::Light);
			TestFalse("4th press during Hit 3 recovery rejected", b4thAccepted);
			TestEqual("Chain index remains 2", CombatComp->GetCurrentChainIndex(), 2);
		});

		It("resets chain to index 0 on return to Idle", [this]()
		{
			// Advance to hit 2
			CombatComp->RequestAction(EHeroAction::Light);
			CombatComp->OpenCancelWindow({ EHeroAction::Light });
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Chain index is 1", CombatComp->GetCurrentChainIndex(), 1);

			// Ending the real montage must invoke its registered completion callback.
			Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
			Hero->GetMesh()->TickAnimation(0.01f, false);
			Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
			TestEqual("Montage end returns to Idle", CombatComp->GetActionState(), EHeroActionState::Idle);
			TestEqual("Chain reset to 0", CombatComp->GetCurrentChainIndex(), 0);

			// Next Light starts at index 0
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Next Light starts at index 0", CombatComp->GetCurrentChainIndex(), 0);
		});

		It("resets chain to index 0 when interrupting action starts", [this]()
		{
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Chain index is 0", CombatComp->GetCurrentChainIndex(), 0);

			// Open cancel window allowing Dodge
			CombatComp->OpenCancelWindow({ EHeroAction::Dodge });
			const bool bDodgeAccepted = CombatComp->RequestAction(EHeroAction::Dodge);
			TestTrue("Dodge accepted from cancel window", bDodgeAccepted);
			TestEqual("State transitioned to Dodge", CombatComp->GetActionState(), EHeroActionState::Dodge);
			TestEqual("Chain index reset to 0 by Dodge", CombatComp->GetCurrentChainIndex(), 0);
			TestFalse("Cancelled light montage is stopped", Hero->GetMesh()->GetAnimInstance()->Montage_IsPlaying(ClassDef->LightChain[0].Montage));
		});
	});

	Describe("Invalid action refusal and window expiry", [this]()
	{
		It("rejects missing hit timing without spending stamina or entering attack state", [this]()
		{
			ClassDef->LightChain[0].Montage = nullptr;
			ClassDef->LightChain[0].StaminaCost = 5.f;
			AddExpectedError(TEXT("Light attack refused:"), EAutomationExpectedErrorFlags::Contains, 1);
			TestFalse("Invalid action rejected", CombatComp->RequestAction(EHeroAction::Light));
			TestFalse("Repeated invalid action rejected", CombatComp->RequestAction(EHeroAction::Light));
			TestEqual("No partial state", CombatComp->GetActionState(), EHeroActionState::Idle);
			TestEqual("No stamina spent", StaminaComp->GetCurrentStamina(), 100.f);
		});

		It("resets chain when an unused chain window closes", [this]()
		{
			CombatComp->RequestAction(EHeroAction::Light);
			CombatComp->OpenCancelWindow({ EHeroAction::Light });
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Second step", CombatComp->GetCurrentChainIndex(), 1);
			CombatComp->OpenCancelWindow({ EHeroAction::Light });
			CombatComp->CloseCancelWindow();
			TestEqual("Expired chain resets", CombatComp->GetCurrentChainIndex(), 0);
		});

		It("rejects overlapping or out-of-bounds notify windows", [this]()
		{
			UAnimMontage* Montage = DuplicateObject<UAnimMontage>(ClassDef->LightChain[0].Montage, ClassDef);
			ClassDef->LightChain[0].Montage = Montage;
			Montage->Notifies.Reset();
			FCombatActionTiming::AddHitWindow(Montage, 0.15f, 0.2f);
			FCombatActionTiming::AddCancelWindow(Montage, 0.2f, 0.5f,
				{ EHeroAction::Light, EHeroAction::Heavy, EHeroAction::Dodge, EHeroAction::BlockStart });
			FString Error;
			TestFalse("Active cannot be cancelled", ClassDef->ValidateLightAttack(0, Error));
			Montage->Notifies.Reset();
			FCombatActionTiming::AddHitWindow(Montage, Montage->GetPlayLength() - 0.1f, 0.5f);
			TestFalse("Notify cannot overrun montage", ClassDef->ValidateLightAttack(0, Error));
		});
	});

	Describe("Damage and Pending Attack Configuration", [this]()
	{
		It("configures MeleeTraceComponent with matching damage for each chain hit", [this]()
		{
			TestNotNull("TraceComp exists", TraceComp);

			// Hit 1: 10 damage
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Hit 1 pending damage is 10", TraceComp->GetPendingAttackTemplate().Damage, 10.f);
			TestEqual("Hit 1 pending poise damage is 5", TraceComp->GetPendingAttackTemplate().PoiseDamage, 5.f);
			TestFalse("Hit 1 is not heavy", TraceComp->GetPendingAttackTemplate().bIsHeavy);
			TestEqual("Hit 1 source layer is Hero", TraceComp->GetPendingAttackTemplate().SourceLayer, ECombatLayer::Hero);

			// Hit 2: 10 damage
			CombatComp->OpenCancelWindow({ EHeroAction::Light });
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Hit 2 pending damage is 10", TraceComp->GetPendingAttackTemplate().Damage, 10.f);

			// Hit 3: 14 damage
			CombatComp->OpenCancelWindow({ EHeroAction::Light });
			CombatComp->RequestAction(EHeroAction::Light);
			TestEqual("Hit 3 pending damage is 14", TraceComp->GetPendingAttackTemplate().Damage, 14.f);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
