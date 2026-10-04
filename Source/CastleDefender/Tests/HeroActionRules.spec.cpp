#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Hero/HeroCombatTypes.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroCharacter.h"
#include "Core/GameTags.h"
#include "Tests/HeroCombatFixture.h"

BEGIN_DEFINE_SPEC(FHeroActionRulesSpec, "CastleDefender.Combat.Hero.ActionRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroActionRulesSpec)

void FHeroActionRulesSpec::Define()
{
	Describe("Pure Action Rules (FHeroActionRules::CanStart)", [this]()
	{
		It("allows starting initial actions from Idle", [this]()
		{
			const TArray<EHeroAction> EmptyWindows;
			TestTrue("Idle -> Light", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Light));
			TestTrue("Idle -> Heavy", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Heavy));
			TestTrue("Idle -> Dodge", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Dodge));
			TestTrue("Idle -> BlockStart", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::BlockStart));
			TestTrue("Idle -> Parry", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Parry));
			TestTrue("Idle -> Interact", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Interact));
			TestFalse("Idle -> BlockEnd", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::BlockEnd));
		});

		It("rejects all actions when Dead", [this]()
		{
			const TArray<EHeroAction> AllActions = {
				EHeroAction::Light, EHeroAction::Heavy, EHeroAction::Dodge,
				EHeroAction::BlockStart, EHeroAction::BlockEnd, EHeroAction::Parry, EHeroAction::Interact
			};

			for (EHeroAction Action : AllActions)
			{
				TestFalse("Dead rejects action", FHeroActionRules::CanStart(EHeroActionState::Dead, AllActions, Action));
			}
		});

		It("rejects all actions when shared Staggered is active", [this]()
		{
			const TArray<EHeroAction> AllActions = {
				EHeroAction::Light, EHeroAction::Heavy, EHeroAction::Dodge,
				EHeroAction::BlockStart, EHeroAction::BlockEnd, EHeroAction::Parry, EHeroAction::Interact
			};

			for (EHeroAction Action : AllActions)
			{
				TestFalse("Staggered rejects action", FHeroActionRules::CanStart(EHeroActionState::Idle, AllActions, Action, true, true));
			}
		});

		It("rejects stamina-consuming actions when stamina is insufficient", [this]()
		{
			const TArray<EHeroAction> EmptyWindows;
			TestFalse("No stamina -> Light", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Light, false));
			TestFalse("No stamina -> Heavy", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Heavy, false));
			TestFalse("No stamina -> Dodge", FHeroActionRules::CanStart(EHeroActionState::Idle, EmptyWindows, EHeroAction::Dodge, false));
		});

		It("enforces commitment during active attacks without cancel windows", [this]()
		{
			const TArray<EHeroAction> NoCancel;
			TestFalse("LightAttack locked", FHeroActionRules::CanStart(EHeroActionState::LightAttack, NoCancel, EHeroAction::Light));
			TestFalse("HeavyAttack locked", FHeroActionRules::CanStart(EHeroActionState::HeavyAttack, NoCancel, EHeroAction::Dodge));
			TestFalse("Dodge locked", FHeroActionRules::CanStart(EHeroActionState::Dodge, NoCancel, EHeroAction::Light));
			TestFalse("Parry locked", FHeroActionRules::CanStart(EHeroActionState::Parry, NoCancel, EHeroAction::Light));
		});

		It("permits transitions when allowed by open cancel windows", [this]()
		{
			const TArray<EHeroAction> LightAndDodge = { EHeroAction::Light, EHeroAction::Dodge };
			TestTrue("Light in cancel window", FHeroActionRules::CanStart(EHeroActionState::LightAttack, LightAndDodge, EHeroAction::Light));
			TestTrue("Dodge in cancel window", FHeroActionRules::CanStart(EHeroActionState::LightAttack, LightAndDodge, EHeroAction::Dodge));
			TestFalse("Heavy not in window", FHeroActionRules::CanStart(EHeroActionState::LightAttack, LightAndDodge, EHeroAction::Heavy));
		});

		It("handles Block release and transitions correctly", [this]()
		{
			const TArray<EHeroAction> NoCancel;
			TestTrue("Block -> BlockEnd allowed", FHeroActionRules::CanStart(EHeroActionState::Block, NoCancel, EHeroAction::BlockEnd));
			TestFalse("Block -> Light disallowed without window", FHeroActionRules::CanStart(EHeroActionState::Block, NoCancel, EHeroAction::Light));

			const TArray<EHeroAction> CounterWindow = { EHeroAction::Light };
			TestTrue("Block -> Light allowed with window", FHeroActionRules::CanStart(EHeroActionState::Block, CounterWindow, EHeroAction::Light));
		});
	});

	Describe("UHeroCombatComponent State and Window Lifecycle", [this]()
	{
		It("maps states to corresponding GameplayTags", [this]()
		{
			FHeroCombatFixture Fixture;
			UHeroCombatComponent* Comp = Fixture.Hero->GetCombatComponent();
			TestEqual(TEXT("Idle tag"), Comp->GetActionTag(), FGameplayTag());

			Comp->RequestAction(EHeroAction::Light);
			TestTrue(TEXT("LightAttack tag"), Comp->GetActionTag() == GameTags::State_Hero_Attacking);

			Comp->OpenCancelWindow({ EHeroAction::Dodge });
			Comp->RequestAction(EHeroAction::Dodge);
			TestTrue(TEXT("Dodge tag"), Comp->GetActionTag() == GameTags::State_Hero_Dodging);

			Comp->OpenCancelWindow({ EHeroAction::BlockStart });
			Comp->RequestAction(EHeroAction::BlockStart);
			TestTrue(TEXT("Block tag"), Comp->GetActionTag() == GameTags::State_Hero_Blocking);

			Comp->RequestAction(EHeroAction::BlockEnd);
			TestEqual(TEXT("Returned to Idle tag"), Comp->GetActionTag(), FGameplayTag());

			Comp->RequestAction(EHeroAction::Parry);
			TestTrue(TEXT("Parry tag"), Comp->GetActionTag() == GameTags::State_Hero_Parrying);
		});

		It("buffers rejected actions and consumes when cancel window opens", [this]()
		{
			FHeroCombatFixture Fixture;
			UHeroCombatComponent* Comp = Fixture.Hero->GetCombatComponent();
			Comp->RequestAction(EHeroAction::Light);
			TestEqual(TEXT("State is LightAttack"), Comp->GetActionState(), EHeroActionState::LightAttack);

			// Early press of Dodge is rejected and buffered
			const bool bImmediate = Comp->RequestAction(EHeroAction::Dodge);
			TestFalse(TEXT("Immediate Dodge rejected"), bImmediate);
			TestEqual(TEXT("State still LightAttack"), Comp->GetActionState(), EHeroActionState::LightAttack);

			// AnimNotify opens cancel window allowing Dodge
			Comp->OpenCancelWindow({ EHeroAction::Dodge });
			TestEqual(TEXT("Buffered Dodge consumed into Dodge state"), Comp->GetActionState(), EHeroActionState::Dodge);
		});

		It("force-closes windows on return to Idle", [this]()
		{
			UHeroCombatComponent* Comp = NewObject<UHeroCombatComponent>();
			Comp->OpenCancelWindow({ EHeroAction::Light });
			Comp->OpenInvulnerableWindow();
			Comp->OpenParryWindow();

			TestTrue("Cancel window open", Comp->IsInCancelWindow());
			TestTrue("Invulnerable window open", Comp->IsInInvulnerableWindow());
			TestTrue("Parry window open", Comp->IsInParryWindow());

			Comp->ForceCloseAllWindows();
			TestFalse("Cancel window closed", Comp->IsInCancelWindow());
			TestFalse("Invulnerable window closed", Comp->IsInInvulnerableWindow());
			TestFalse("Parry window closed", Comp->IsInParryWindow());
		});
	});
}

#endif
