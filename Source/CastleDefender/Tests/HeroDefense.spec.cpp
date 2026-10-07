#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Tests/CombatTestListener.h"
#include "Tests/FeedbackTestListener.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroAnimInstance.h"
#include "Hero/StaminaComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Core/GameTags.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace HeroDefenseSpec
{
	/** Hero in Block with full stamina and HP 200; feedback goes to a transient table so the counts are exact. */
	struct FBlockFixture : FHeroCombatFixture
	{
		UHeroCombatComponent* Combat = nullptr;
		UStaminaComponent* Stamina = nullptr;
		UHealthComponent* Health = nullptr;
		UFeedbackTestListener* Feedback = nullptr;
		UCombatTestListener* Resolutions = nullptr;

		FBlockFixture()
		{
			BeginPlay();
			Combat = Hero->GetCombatComponent();
			Stamina = Hero->GetStaminaComponent();
			Health = Hero->GetHealthComponent();
			Health->InitializeHealth(200.f, 0.f);
			UFeedbackSubsystem* Subsystem = UFeedbackSubsystem::Get(World);
			Subsystem->SetFeedbackTable(UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Block, FeedbackTags::Combat_BlockBreak,
				FeedbackTags::State_Staggered_Applied, FeedbackTags::State_Staggered_Removed }));
			Feedback = NewObject<UFeedbackTestListener>();
			Subsystem->OnFeedbackPlayed.AddDynamic(Feedback, &UFeedbackTestListener::HandlePlayed);
			Resolutions = NewObject<UCombatTestListener>();
			Combat->OnCombatResolved.AddDynamic(Resolutions, &UCombatTestListener::HandleCombatResolved);
		}

		ECombatHitResult Hit(float Damage, bool bFromFront) const
		{
			FCombatHit CombatHit;
			CombatHit.Damage = Damage;
			// Direction of travel: a frontal attacker swings toward the hero's face.
			CombatHit.HitDirection = bFromFront ? -Hero->GetActorForwardVector() : Hero->GetActorForwardVector();
			return UCombatLibrary::DeliverHit(Hero, CombatHit);
		}

		void EndMontages() const
		{
			Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
			Hero->GetMesh()->TickAnimation(0.01f, false);
			Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
		}
	};
}

BEGIN_DEFINE_SPEC(FHeroDefenseSpec, "CastleDefender.Combat.Hero.Block", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroDefenseSpec)

void FHeroDefenseSpec::Define()
{
	using namespace HeroDefenseSpec;

	It("reduces a frontal hit, drains stamina from the original force and stays in Block (AC-CMB-08)", [this]()
	{
		FBlockFixture F;
		TestTrue("Block accepted", F.Combat->RequestAction(EHeroAction::BlockStart));
		TestEqual("Blocked", F.Hit(20.f, true), ECombatHitResult::Blocked);
		TestEqual("HP -4", F.Health->GetCurrentHealth(), 196.f);
		TestEqual("Stamina 80", F.Stamina->GetCurrentStamina(), 80.f);
		TestEqual("Still blocking", F.Combat->GetActionState(), EHeroActionState::Block);
		TestEqual("One block feedback", F.Feedback->PlayedCount, 1);
		TestEqual("Block feedback tag", F.Feedback->LastPlayed, FGameplayTag(FeedbackTags::Combat_Block));
		TestEqual("One resolution", F.Resolutions->CombatResolvedCount, 1);
		TestTrue("Resolution marks the hit blocked", F.Resolutions->LastResolutionEvent.Hit.bWasBlocked);
		TestEqual("Resolution keeps the original force", F.Resolutions->LastResolutionEvent.Hit.Damage, 20.f);
		if (UAnimMontage* BlockHit = F.Hero->GetHeroClassDefinition()->Block.BlockHitMontage)
		{
			TestTrue("Block-hit reaction plays", F.Hero->GetMesh()->GetAnimInstance()->Montage_IsActive(BlockHit));
		}
		else
		{
			AddError(TEXT("DA_HeroClass_Warlord has no Block.BlockHitMontage; run Tools/create_hero_assets.bat"));
		}
	});

	It("does not block a hit from behind (AC-CMB-08)", [this]()
	{
		FBlockFixture F;
		F.Combat->RequestAction(EHeroAction::BlockStart);
		TestEqual("Normal hit", F.Hit(20.f, false), ECombatHitResult::Hit);
		TestEqual("Full damage", F.Health->GetCurrentHealth(), 180.f);
		TestEqual("Stamina unchanged", F.Stamina->GetCurrentStamina(), 100.f);
		TestEqual("Back hit reaction leaves Block", F.Combat->GetActionState(), EHeroActionState::HitReact);
		TestFalse("Blocking regen off", F.Stamina->IsBlocking());
		TestEqual("No block feedback", F.Feedback->PlayedCount, 0);
	});

	It("breaks the guard into shared Staggered at 0 stamina and gates every action (AC-CMB-09, AC-CMB-22)", [this]()
	{
		FBlockFixture F;
		auto* Breaks = NewObject<UCombatTestListener>();
		F.Combat->OnBlockBroken.AddDynamic(Breaks, &UCombatTestListener::HandleBlockBroken);
		F.Stamina->ApplyDamage(90.f);
		F.Combat->RequestAction(EHeroAction::BlockStart);
		TestEqual("Block broken", F.Hit(20.f, true), ECombatHitResult::BlockBroken);
		TestEqual("Breaking hit still reduced", F.Health->GetCurrentHealth(), 196.f);
		TestEqual("Stamina empty", F.Stamina->GetCurrentStamina(), 0.f);
		TestEqual("Guard dropped", F.Combat->GetActionState(), EHeroActionState::Idle);
		UCombatStateComponent* States = F.Hero->GetCombatStateComponent();
		TestTrue("Shared Staggered", States->HasState(GameTags::State_Combat_Staggered));
		TestEqual("Staggered for BlockBreakStaggerDuration", States->GetStateRemaining(GameTags::State_Combat_Staggered),
			F.Hero->GetHeroClassDefinition()->Block.BlockBreakStaggerDuration, 0.01f);
		TestEqual("OnBlockBroken once", Breaks->BlockBrokenCount, 1);
		// Staggered.Applied is the shared state's own presentation; the hit outcome adds exactly one BlockBreak row.
		TestEqual("One BlockBreak feedback", F.Feedback->PlayedTags.FilterByPredicate(
			[](const FGameplayTag& Tag) { return Tag == FeedbackTags::Combat_BlockBreak; }).Num(), 1);
		TestFalse("No Block feedback", F.Feedback->PlayedTags.Contains(FeedbackTags::Combat_Block));
		TestFalse("Light refused", F.Combat->RequestAction(EHeroAction::Light));
		TestFalse("Dodge refused", F.Combat->RequestAction(EHeroAction::Dodge));
		TestFalse("Block refused", F.Combat->RequestAction(EHeroAction::BlockStart));
		TestFalse("Nothing buffered while Staggered", F.Combat->HasBufferedInput());
		UAnimInstance* Anim = F.Hero->GetMesh()->GetAnimInstance();
		UAnimMontage* BreakMontage = F.Hero->GetHeroClassDefinition()->Block.BlockBreakMontage;
		TestTrue("Block-break reaction plays", BreakMontage && Anim->Montage_IsActive(BreakMontage));

		// Expiry belongs to SYN; removing the state is what its timer does.
		States->RemoveState(GameTags::State_Combat_Staggered);
		TestTrue("Break reaction stops with the state", !BreakMontage || Anim->Montage_GetIsStopped(BreakMontage));
		TestEqual("Held Block resumes once Staggered ends", F.Combat->GetActionState(), EHeroActionState::Block);
	});

	It("returns to Idle on release in the same frame and restores speed and regen", [this]()
	{
		FBlockFixture F;
		F.Hero->GetMesh()->SetAnimInstanceClass(UHeroAnimInstance::StaticClass());
		const UHeroAnimInstance* Anim = Cast<UHeroAnimInstance>(F.Hero->GetMesh()->GetAnimInstance());
		if (!TestNotNull("Hero anim instance", Anim)) { return; }
		const float JogSpeed = F.Hero->GetHeroClassDefinition()->Movement.JogSpeed;
		F.Combat->RequestAction(EHeroAction::BlockStart);
		TestTrue("Blocking regen on", F.Stamina->IsBlocking());
		TestEqual("Guard speed", F.Hero->GetCharacterMovement()->MaxWalkSpeed,
			JogSpeed * F.Hero->GetHeroClassDefinition()->Block.MoveSpeedMultiplier);
		F.Hero->GetMesh()->TickAnimation(0.01f, false);
		TestTrue("Guard pose flag", Anim->IsBlocking());
		TestTrue("Guard pose fading in", Anim->GetGuardAlpha() > 0.f && Anim->GetGuardAlpha() < 1.f);
		F.Hero->GetMesh()->TickAnimation(0.2f, false);
		TestEqual("Guard pose raised", Anim->GetGuardAlpha(), 1.f);
		TestTrue("Release accepted", F.Combat->RequestAction(EHeroAction::BlockEnd));
		TestEqual("Idle immediately", F.Combat->GetActionState(), EHeroActionState::Idle);
		F.Hero->GetMesh()->TickAnimation(0.01f, false);
		TestFalse("Guard pose flag cleared", Anim->IsBlocking());
		TestFalse("Blocking regen off", F.Stamina->IsBlocking());
		TestEqual("Jog speed restored", F.Hero->GetCharacterMovement()->MaxWalkSpeed, JogSpeed);
		TestEqual("Unblocked hit is full damage", F.Hit(20.f, true), ECombatHitResult::Hit);
	});

	It("resumes a Block held through an attack instead of buffering it", [this]()
	{
		FBlockFixture F;
		TestTrue("Light", F.Combat->RequestAction(EHeroAction::Light));
		TestFalse("Block refused mid-swing", F.Combat->RequestAction(EHeroAction::BlockStart));
		TestFalse("Block is not buffered", F.Combat->HasBufferedInput());
		F.EndMontages();
		TestEqual("Held Block starts when the swing ends", F.Combat->GetActionState(), EHeroActionState::Block);

		F.Combat->RequestAction(EHeroAction::BlockEnd);
		F.Combat->RequestAction(EHeroAction::Light);
		F.Combat->RequestAction(EHeroAction::BlockStart);
		F.Combat->RequestAction(EHeroAction::BlockEnd);
		F.EndMontages();
		TestEqual("Released before the end: no Block", F.Combat->GetActionState(), EHeroActionState::Idle);
	});

	It("restarts blocking-regen suppression on every absorbed hit, then regenerates at the blocking rate (R-CMB-49)", [this]()
	{
		FBlockFixture F;
		const FStaminaConfig& Config = F.Hero->GetHeroClassDefinition()->Stamina;
		// Suppression longer than the regen delay, so only a restarted suppression can hold regen back.
		const float Suppression = Config.RegenDelay + 0.2f;
		F.Hero->GetHeroClassDefinition()->Block.BlockRegenSuppressAfterHit = Suppression;
		F.Combat->RequestAction(EHeroAction::BlockStart);
		F.Hit(10.f, true);
		F.Stamina->TickComponent(0.5f, LEVELTICK_All, nullptr);
		F.Hit(10.f, true);
		const float AfterSecondHit = F.Stamina->GetCurrentStamina();
		TestEqual("Two blocked hits", AfterSecondHit, 80.f);
		// Past the first hit's suppression and the second hit's regen delay, but inside the second suppression.
		F.Stamina->TickComponent(Suppression - 0.05f, LEVELTICK_All, nullptr);
		TestEqual("Second hit restarted suppression", F.Stamina->GetCurrentStamina(), AfterSecondHit);
		F.Stamina->TickComponent(0.05f, LEVELTICK_All, nullptr);
		F.Stamina->TickComponent(1.f, LEVELTICK_All, nullptr);
		TestEqual("Blocking regen rate", F.Stamina->GetCurrentStamina(),
			AfterSecondHit + Config.RegenRate * Config.BlockingRegenMultiplier, 0.01f);
	});
}
#endif
