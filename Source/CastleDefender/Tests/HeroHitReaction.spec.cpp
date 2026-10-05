#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Tests/CombatTestListener.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Core/GameTags.h"
#include "Feedback/FeedbackTags.h"
#include "GameFramework/CharacterMovementComponent.h"

BEGIN_DEFINE_SPEC(FHeroHitReactionSpec, "CastleDefender.Combat.Hero.HitReaction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroHitReactionSpec)

void FHeroHitReactionSpec::Define()
{
    It("interrupts a swing, selects front/back, and completes back to Idle", [this]()
    {
        FHeroCombatFixture F;
        F.BeginPlay();
        auto* Combat = F.Hero->GetCombatComponent();
        auto* Anim = F.Hero->GetMesh()->GetAnimInstance();
        auto* Listener = NewObject<UCombatTestListener>();
        Combat->OnCombatResolved.AddDynamic(Listener, &UCombatTestListener::HandleCombatResolved);
        Combat->RequestAction(EHeroAction::Light);
        Combat->RequestAction(EHeroAction::Dodge);
        F.Hero->GetMeleeTraceComponent()->BeginHitWindow();
        FCombatHit Hit;
        Hit.Damage = 20.f;
        Hit.HitDirection = -F.Hero->GetActorForwardVector();
        Hit.bWasBlocked = true; // DeliverHit must replace attacker-supplied outcome metadata.
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestFalse("Resolution also sanitizes attacker outcome metadata", Listener->LastResolutionEvent.Hit.bWasBlocked);
        TestEqual("HitReact owns action", Combat->GetActionState(), EHeroActionState::HitReact);
        TestTrue("Front montage", Anim->Montage_IsActive(F.Hero->GetHeroClassDefinition()->HitReact.FrontMontage));
        TestFalse("Trace closed", F.Hero->GetMeleeTraceComponent()->IsHitWindowActive());
        TestFalse("Buffer cleared", Combat->HasBufferedInput());
        Hit.HitDirection = F.Hero->GetActorForwardVector();
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        F.Hero->GetMesh()->TickAnimation(0.01f, false);
        Anim->DispatchQueuedAnimEvents();
        TestEqual("Old montage end cannot reset restarted reaction", Combat->GetActionState(), EHeroActionState::HitReact);
        TestTrue("Back montage", Anim->Montage_IsActive(F.Hero->GetHeroClassDefinition()->HitReact.BackMontage));
        Anim->Montage_Stop(0.f);
        F.Hero->GetMesh()->TickAnimation(0.01f, false);
        Anim->DispatchQueuedAnimEvents();
        TestEqual("Reaction ends", Combat->GetActionState(), EHeroActionState::Idle);
    });

    It("requires enabled authored resistance and never prevents damage or shared Staggered", [this]()
    {
        FHeroCombatFixture F;
        F.BeginPlay();
        auto* Combat = F.Hero->GetCombatComponent();
        auto* Def = F.Hero->GetHeroClassDefinition();
        FCombatHit Hit;
        Hit.Damage = 10.f;
        Hit.InterruptData.InterruptStrength = 1.f;
        Combat->RequestAction(EHeroAction::Heavy);
        Combat->OpenInterruptResistanceWindow();
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestEqual("Default off interrupts", Combat->GetActionState(), EHeroActionState::HitReact);
        Combat->OpenCancelWindow({EHeroAction::Dodge});
        Combat->RequestAction(EHeroAction::Dodge);
        Combat->ForceCloseAllWindows();
        F.Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
        F.Hero->GetMesh()->TickAnimation(0.01f, false);
        F.Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
        Def->Heavy.bInterruptResistanceEnabled = true;
        Def->Heavy.InterruptResistance = 2.f;
        Combat->RequestAction(EHeroAction::Heavy);
        Combat->OpenInterruptResistanceWindow();
        const float HP = F.Hero->GetHealthComponent()->GetCurrentHealth();
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestEqual("Weak interrupt resisted", Combat->GetActionState(), EHeroActionState::HeavyAttack);
        TestEqual("Damage still applied", F.Hero->GetHealthComponent()->GetCurrentHealth(), HP - 10.f);
        Hit.InterruptData.InterruptStrength = 2.f;
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestEqual("Equal threshold interrupts", Combat->GetActionState(), EHeroActionState::HitReact);
        F.Hero->GetCombatStateComponent()->ApplyState(GameTags::State_Combat_Staggered, 1.f, nullptr);
        TestFalse("Shared state gates actions", Combat->CanStartAction(EHeroAction::Dodge));
        TestFalse("Shared interruption closes cancels", Combat->IsInCancelWindow());
    });

    It("skips blocked damage reactions and permits only late Dodge cancellation", [this]()
    {
        FHeroCombatFixture F;
        F.BeginPlay();
        auto* Combat = F.Hero->GetCombatComponent();
        Combat->RequestAction(EHeroAction::Light);
        FCombatHit Hit;
        Hit.Damage = 10.f;
        Hit.bWasBlocked = true;
        F.Hero->GetHealthComponent()->OnDamaged.Broadcast(Hit, 190.f);
        TestEqual("Resolved blocked damage keeps action", Combat->GetActionState(), EHeroActionState::LightAttack);
        Hit.bWasBlocked = false;
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestFalse("Early dodge cannot cancel", Combat->CanStartAction(EHeroAction::Dodge));
        Combat->OpenCancelWindow({EHeroAction::Dodge});
        TestFalse("Light remains forbidden", Combat->CanStartAction(EHeroAction::Light));
        TestTrue("Late dodge accepted", Combat->RequestAction(EHeroAction::Dodge));
    });

    It("dies once, closes windows and input, and emits one death feedback request", [this]()
    {
        FHeroCombatFixture F;
        F.BeginPlay();
        auto* Combat = F.Hero->GetCombatComponent();
        auto* Listener = NewObject<UCombatTestListener>();
        F.Hero->OnHeroDeath.AddDynamic(Listener, &UCombatTestListener::HandleDeath);
        Listener->HeroCombat = Combat;
        F.Hero->OnFeedbackRequested.AddDynamic(Listener, &UCombatTestListener::HandleFeedback);
        F.Hero->StartSprint();
        Combat->RequestAction(EHeroAction::Light);
        Combat->RequestAction(EHeroAction::Dodge);
        FCombatHit Hit;
        Hit.Damage = 1000.f;
        TestTrue("Death handler bound", F.Hero->GetHealthComponent()->OnDeath.IsBound());
        TestEqual("Lethal resolution", UCombatLibrary::DeliverHit(F.Hero, Hit), ECombatHitResult::Killed);
        TestTrue("Health dead", F.Hero->GetHealthComponent()->IsDead());
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestEqual("Dead", Combat->GetActionState(), EHeroActionState::Dead);
        TestEqual("One death", Listener->DeathCount, 1);
        TestEqual("Observers see committed Dead state", Listener->ActionStateAtDeath, EHeroActionState::Dead);
        TestEqual("One feedback", Listener->FeedbackCount, 1);
        TestEqual("Death tag", Listener->LastFeedback, FGameplayTag(FeedbackTags::Hero_Death));
        TestFalse("No buffered action", Combat->HasBufferedInput());
        TestFalse("No new action", Combat->RequestAction(EHeroAction::Light));
        F.Hero->StartSprint();
        TestFalse("Dead cannot sprint", F.Hero->IsSprinting());
        TestEqual("Movement disabled", F.Hero->GetCharacterMovement()->MovementMode, MOVE_None);
    });

    It("keeps Dead when death presentation cannot play", [this]()
    {
        FHeroCombatFixture F;
        F.BeginPlay();
        F.Hero->GetMesh()->SetAnimInstanceClass(nullptr);
        AddExpectedError(TEXT("Cannot play action montage"), EAutomationExpectedErrorFlags::Contains, 1);
        FCombatHit Hit;
        Hit.Damage = 1000.f;
        UCombatLibrary::DeliverHit(F.Hero, Hit);
        TestEqual("Presentation failure cannot revive action", F.Hero->GetCombatComponent()->GetActionState(), EHeroActionState::Dead);
    });
}
#endif
