#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/TestDummy.h"
#include "Core/GameTags.h"
#include "Hero/HeroCombatLibrary.h"
#include "Hero/StaminaComponent.h"
#include "Tests/CombatTestListener.h"
#include "Tests/FeedbackTestListener.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"

namespace
{
	struct FParryFixture : FHeroCombatFixture
	{
		ATestDummy* Attacker = nullptr;
		FParryFixture()
		{
			BeginPlay();
			Attacker = World->SpawnActor<ATestDummy>(FVector(150.f, 0.f, 0.f), FRotator::ZeroRotator);
			Attacker->DispatchBeginPlay();
		}
		FCombatHit Incoming() const
		{
			FCombatHit Hit; Hit.Damage = 20.f; Hit.Instigator = Attacker; Hit.SourceLayer = ECombatLayer::Enemy;
			return Hit;
		}
		bool Start() const
		{
			const bool bStarted = Hero->GetCombatComponent()->RequestAction(EHeroAction::Parry);
			if (bStarted) { Hero->GetCombatComponent()->OpenParryWindow(); }
			return bStarted;
		}
	};
}

BEGIN_DEFINE_SPEC(FParrySpec, "CastleDefender.Combat.Hero.Parry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FParrySpec)

void FParrySpec::Define()
{
	It("does not parry a hit outside the frontal arc", [this]()
	{
		FHeroCombatFixture F;
		F.BeginPlay();
		TestTrue("Parry starts", F.Hero->GetCombatComponent()->RequestAction(EHeroAction::Parry));
		F.Hero->GetCombatComponent()->OpenParryWindow();
		FCombatHit Hit;
		Hit.Damage = 20.f;
		Hit.SourceLayer = ECombatLayer::Enemy;
		Hit.HitDirection = F.Hero->GetActorForwardVector();
		TestEqual("Rear hit is not parried", UCombatLibrary::DeliverHit(F.Hero, Hit), ECombatHitResult::Hit);
		TestEqual("Damage applies", F.Hero->GetHealthComponent()->GetCurrentHealth(), 180.f);
	});
	It("consumes one success, applies attacker poise, opens the counter and emits one Parry feedback", [this]()
	{
		FParryFixture F;
		auto* Listener = NewObject<UCombatTestListener>();
		F.Hero->GetCombatComponent()->OnParrySucceeded.AddDynamic(Listener, &UCombatTestListener::HandleParrySucceeded);
		auto* Feedback = NewObject<UFeedbackTestListener>();
		auto* Subsystem = UFeedbackSubsystem::Get(F.World);
		Subsystem->SetFeedbackTable(UFeedbackTestListener::MakeTable({FeedbackTags::Combat_Parry, FeedbackTags::State_Staggered_Applied,
			FeedbackTags::Combat_Hit_Light, FeedbackTags::Hero_Damaged}));
		Subsystem->OnFeedbackPlayed.AddDynamic(Feedback, &UFeedbackTestListener::HandlePlayed);
		TestTrue("Parry starts", F.Start());
		TestEqual("First hit parried", UCombatLibrary::DeliverHit(F.Hero, F.Incoming()), ECombatHitResult::Parried);
		TestEqual("No damage", F.Hero->GetHealthComponent()->GetCurrentHealth(), 200.f);
		TestTrue("Poise break", F.Attacker->GetCombatState()->HasState(GameTags::State_Combat_Staggered));
		TestEqual("One success", Listener->ParrySucceededCount, 1);
		TestEqual("Attacker identity", Listener->ParryAttacker, static_cast<AActor*>(F.Attacker));
		TestTrue("Counter ready", F.Hero->GetCombatComponent()->HasCounterWindow());
		TestTrue("Consumed", F.Hero->GetCombatComponent()->IsParryConsumed());
		TestFalse("Parry window closes", F.Hero->GetCombatComponent()->IsInParryWindow());
		TestEqual("One Parry feedback", Feedback->PlayedTags.FilterByPredicate([](const FGameplayTag& Tag) { return Tag == FeedbackTags::Combat_Parry; }).Num(), 1);
		TestEqual("Second same-frame hit normal", UCombatLibrary::DeliverHit(F.Hero, F.Incoming()), ECombatHitResult::Hit);
		TestEqual("Only second damages", F.Hero->GetHealthComponent()->GetCurrentHealth(), 180.f);
		TestEqual("Still one success", Listener->ParrySucceededCount, 1);
	});
	It("consumes before a success listener delivers a reentrant hit", [this]()
	{
		FParryFixture F;
		auto* Listener = NewObject<UCombatTestListener>();
		Listener->ReentrantParryTarget = F.Hero;
		F.Hero->GetCombatComponent()->OnParrySucceeded.AddDynamic(Listener, &UCombatTestListener::HandleParrySucceeded);
		TestTrue("Parry starts", F.Start());
		TestEqual("Original parried", UCombatLibrary::DeliverHit(F.Hero, F.Incoming()), ECombatHitResult::Parried);
		TestEqual("Nested hit normal", Listener->ReentrantParryResult, ECombatHitResult::Hit);
		TestEqual("One success", Listener->ParrySucceededCount, 1);
		TestEqual("Nested damage applies", F.Hero->GetHealthComponent()->GetCurrentHealth(), 180.f);
	});
	It("uses the hero clock once for counter expiry and preserves buffered counter input during hit stop", [this]()
	{
		FParryFixture F;
		auto* Combat = F.Hero->GetCombatComponent();
		TestTrue("Starts", F.Start());
		TestFalse("Early Light buffered", Combat->RequestAction(EHeroAction::Light));
		F.Hero->CustomTimeDilation = 0.1f;
		Combat->TickComponent(0.01f, LEVELTICK_All, nullptr);
		TestTrue("Buffer survives dilated frame", Combat->HasBufferedInput());
		UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		TestEqual("Buffered counter starts on success", Combat->GetActionState(), EHeroActionState::LightAttack);
		TestTrue("Counter payload", F.Hero->GetMeleeTraceComponent()->GetPendingAttackTemplate().bIsParryCounter);
	});
	It("expires an unused counter on the hero clock and disables idle ticking", [this]()
	{
		FParryFixture F;
		auto* Combat = F.Hero->GetCombatComponent();
		F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		F.Hero->CustomTimeDilation = 0.1f;
		Combat->TickComponent(0.01f, LEVELTICK_All, nullptr);
		TestEqual("No double dilation", Combat->GetCounterTimeRemaining(), 0.99f, 0.001f);
		Combat->TickComponent(1.f, LEVELTICK_All, nullptr);
		TestFalse("Expired", Combat->HasCounterWindow());
		TestFalse("No idle clock", Combat->IsComponentTickEnabled());
	});
	It("amplifies only the first counter hit, including a reentrant second target", [this]()
	{
		FParryFixture F;
		auto* Combat = F.Hero->GetCombatComponent();
		F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		TestTrue("Counter Light starts", Combat->RequestAction(EHeroAction::Light));
		auto* Trace = F.Hero->GetMeleeTraceComponent();
		ATestDummy* Other = F.World->SpawnActor<ATestDummy>(FVector(150.f, 70.f, 0.f), FRotator::ZeroRotator);
		Other->DispatchBeginPlay();
		auto* First = NewObject<UCombatTestListener>();
		auto* Second = NewObject<UCombatTestListener>();
		First->ReentrantCounterTrace = Trace; First->ReentrantCounterTarget = Other;
		F.Attacker->GetHealth()->OnDamaged.AddDynamic(First, &UCombatTestListener::HandleCounterDamage);
		Other->GetHealth()->OnDamaged.AddDynamic(Second, &UCombatTestListener::HandleCounterDamage);
		Trace->BeginHitWindow();
		Trace->TryHitTarget(F.Attacker);
		TestEqual("Counter damage", F.Attacker->GetHealth()->GetCurrentHealth(), 85.f);
		TestEqual("Nested normal damage", Other->GetHealth()->GetCurrentHealth(), 90.f);
		TestTrue("First flagged", First->bCounterDamageSeen);
		TestFalse("Second not flagged", Second->bCounterDamageSeen);
		TestEqual("Window consumed before nested hit", Combat->GetCounterTimeRemaining(), 0.f);
	});
	It("has punishable whiff recovery and validates timing before any stamina spend", [this]()
	{
		FParryFixture F;
		auto* Combat = F.Hero->GetCombatComponent();
		F.Start(); Combat->CloseParryWindow();
		TestFalse("No Block in recovery", Combat->RequestAction(EHeroAction::BlockStart));
		TestFalse("No Dodge in recovery", Combat->RequestAction(EHeroAction::Dodge));
		TestEqual("Still committed", Combat->GetActionState(), EHeroActionState::Parry);
		FCombatActionTiming Timing;
		FCombatActionTiming::InspectMontage(F.Hero->GetHeroClassDefinition()->Parry.Montage, Timing);
		TestFalse("No recovery cancel", Timing.bHasCancelWindow);
		TestTrue("Recovery follows active", Timing.ParryWindowEnd < Timing.TotalDuration);
	});
	It("rejects duplicate parry windows and malformed counter data", [this]()
	{
		FParryFixture F;
		auto* Def = F.Hero->GetHeroClassDefinition();
		Def->Parry.Montage = DuplicateObject<UAnimMontage>(Def->Parry.Montage, Def);
		UHeroCombatLibrary::AddParryWindowToMontage(Def->Parry.Montage, 0.1f, 0.1f);
		FString Error;
		TestFalse("Duplicate rejected", Def->ValidateParry(Error));
		AddExpectedError(TEXT("Parry refused:"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse("Does not start", F.Hero->GetCombatComponent()->RequestAction(EHeroAction::Parry));
		TestEqual("Stamina intact", F.Hero->GetStaminaComponent()->GetCurrentStamina(), 100.f);
		Def->Parry.Montage = LoadObject<UAnimMontage>(nullptr, TEXT("/Game/CastleDefender/Hero/AM_Warlord_Parry"));
		Def->Parry.CounterMontage = Def->Dodge.BackwardMontage;
		TestFalse("Defensive montage cannot become a counter attack", Def->ValidateParry(Error));
	});
	It("stops a running sender sweep when parry poise interrupts its hit window", [this]()
	{
		FParryFixture F;
		F.Attacker->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		UMeleeTraceComponent* Sender = NewObject<UMeleeTraceComponent>(F.Attacker);
		F.Attacker->AddInstanceComponent(Sender);
		Sender->RegisterComponent();
		Sender->SetPendingAttack(F.Incoming(), 30.f);
		auto* Listener = NewObject<UCombatTestListener>();
		Listener->TraceToCloseOnState = Sender;
		F.Attacker->GetCombatState()->OnStateAdded.AddDynamic(Listener, &UCombatTestListener::HandleCloseTraceOnState);
		TestTrue("Parry starts", F.Start());
		Sender->BeginHitWindow();
		Sender->TickComponent(0.01f, LEVELTICK_All, nullptr);
		TestFalse("Sender closed inside callback", Sender->IsHitWindowActive());
		TestTrue("One hit parried", F.Hero->GetCombatComponent()->IsParryConsumed());
		TestEqual("Hero undamaged", F.Hero->GetHealthComponent()->GetCurrentHealth(), 200.f);
	});
	It("does not continue an old sweep or overwrite samples when a callback opens a new window", [this]()
	{
		FParryFixture F;
		F.Attacker->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		UMeleeTraceComponent* Sender = NewObject<UMeleeTraceComponent>(F.Attacker);
		F.Attacker->AddInstanceComponent(Sender); Sender->RegisterComponent();
		Sender->SetPendingAttack(F.Incoming(), 30.f);
		auto* Listener = NewObject<UCombatTestListener>();
		Listener->TraceToCloseOnState = Sender; Listener->bReopenTraceOnState = true;
		F.Attacker->GetCombatState()->OnStateAdded.AddDynamic(Listener, &UCombatTestListener::HandleCloseTraceOnState);
		F.Start(); Sender->BeginHitWindow();
		Sender->TickComponent(0.01f, LEVELTICK_All, nullptr);
		TestTrue("New window remains active", Sender->IsHitWindowActive());
		TestEqual("Old sweep does no further damage", F.Hero->GetHealthComponent()->GetCurrentHealth(), 200.f);
		Sender->TickComponent(0.01f, LEVELTICK_All, nullptr);
		TestEqual("New samples do not sweep from the old origin", F.Hero->GetHealthComponent()->GetCurrentHealth(), 200.f);
	});
}
#endif
