#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Tests/CombatTestListener.h"
#include "Tests/FeedbackTestListener.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/TestDummy.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Components/PoseableMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformProcess.h"
#include "Core/GameTags.h"
#include "Animation/AnimData/IAnimationDataModel.h"

namespace
{
	struct FPerfectFixture : FHeroCombatFixture
	{
		FPerfectFixture()
		{
			Hero->GetHeroClassDefinition()->Dodge.bEnablePerfectDodge = true;
			BeginPlay();
			UFeedbackSubsystem::Get(Hero)->SetFeedbackTable(UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_PerfectDodge, FeedbackTags::Combat_Hit_Light, FeedbackTags::Hero_Damaged, FeedbackTags::Hero_Death }));
		}
		UHeroCombatComponent* Combat() const { return Hero->GetCombatComponent(); }
		bool Start() const { const bool Accepted = Combat()->RequestAction(EHeroAction::Dodge); if (Accepted) { Combat()->OpenInvulnerableWindow(); } return Accepted; }
		FCombatHit Incoming() const { FCombatHit Hit; Hit.Damage = 20.f; Hit.PoiseDamage = 50.f; Hit.SourceLayer = ECombatLayer::Enemy; return Hit; }
	};
}
BEGIN_DEFINE_SPEC(FPerfectDodgeSpec, "CastleDefender.Combat.Hero.PerfectDodge", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FPerfectDodgeSpec)

void FPerfectDodgeSpec::Define()
{
	It("awards once before reentrant callbacks, keeps commitment and emits one resolver cue", [this]()
	{
		FPerfectFixture F;
		auto* Combat = F.Combat();
		auto* Listener = NewObject<UCombatTestListener>();
		Listener->ReentrantParryTarget = F.Hero;
		Combat->OnPerfectDodgeSucceeded.AddDynamic(Listener, &UCombatTestListener::HandleParrySucceeded);
		Combat->OnCombatResolved.AddDynamic(Listener, &UCombatTestListener::HandleCombatResolved);
		auto* Cues = NewObject<UFeedbackTestListener>();
		UFeedbackSubsystem::Get(F.Hero)->OnFeedbackPlayed.AddDynamic(Cues, &UFeedbackTestListener::HandlePlayed);
		TestTrue("Dodge starts", F.Start());
		TestEqual("Evaded", UCombatLibrary::DeliverHit(F.Hero, F.Incoming()), ECombatHitResult::Evaded);
		TestEqual("Committed Dodge retained", Combat->GetActionState(), EHeroActionState::Dodge);
		TestEqual("HP unchanged", F.Hero->GetHealthComponent()->GetCurrentHealth(), 200.f);
		TestEqual("Poise unchanged", F.Hero->GetCombatStateComponent()->GetCurrentPoise(), F.Hero->GetCombatStateComponent()->GetMaxPoise());
		TestEqual("One reward", Listener->ParrySucceededCount, 1);
		TestEqual("Nested normal evade", Listener->ReentrantParryResult, ECombatHitResult::Evaded);
		TestEqual("Two resolutions", Listener->CombatResolvedCount, 2);
		TestTrue("Resolver metadata", Listener->LastResolutionEvent.Hit.bWasPerfectDodged);
		TestEqual("One cue", Cues->PlayedCount, 1);
		TestTrue("Counter ready", Combat->HasCounterWindow());
		Combat->CloseInvulnerableWindow(); Combat->OpenInvulnerableWindow();
		UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		TestEqual("Notify reopen does not rearm", Listener->ParrySucceededCount, 1);
	});
	It("late i-frame hits evade silently, and expired eligibility cannot be rearmed", [this]()
	{
		FPerfectFixture F; TestTrue("Started", F.Start());
		F.Hero->CustomTimeDilation = 0.1f;
		F.Combat()->TickComponent(0.02f, LEVELTICK_All, nullptr);
		TestEqual("Hero clock, no double dilation", F.Combat()->GetPerfectDodgeTimeRemaining(), 0.10f, 0.001f);
		F.Combat()->TickComponent(0.11f, LEVELTICK_All, nullptr);
		TestEqual("Late hit evaded", UCombatLibrary::DeliverHit(F.Hero, F.Incoming()), ECombatHitResult::Evaded);
		TestFalse("No reward", F.Combat()->HasCounterWindow());
		F.Combat()->CloseInvulnerableWindow(); F.Combat()->OpenInvulnerableWindow();
		TestEqual("Still expired", F.Combat()->GetPerfectDodgeTimeRemaining(), 0.f);
		TestFalse("Inactive clock disabled", F.Combat()->IsComponentTickEnabled());
	});
	It("empty, zero-damage, environmental and friendly attempts cannot award success or forged metadata", [this]()
	{
		FPerfectFixture F; TestTrue("Started", F.Start()); TestFalse("No hit, no reward", F.Combat()->HasCounterWindow());
		auto* Listener = NewObject<UCombatTestListener>();
		F.Combat()->OnCombatResolved.AddDynamic(Listener, &UCombatTestListener::HandleCombatResolved);
		FCombatHit Hit = F.Incoming(); Hit.Damage = 0.f; Hit.bWasPerfectDodged = true;
		UCombatLibrary::DeliverHit(F.Hero, Hit);
		TestFalse("Forged outcome cleared", Listener->LastResolutionEvent.Hit.bWasPerfectDodged);
		Hit.Damage = 20.f; Hit.SourceLayer = ECombatLayer::Environment; UCombatLibrary::DeliverHit(F.Hero, Hit);
		TestFalse("Environment no reward", F.Combat()->HasCounterWindow());
		Hit = F.Incoming(); Hit.Instigator = F.World->SpawnActor<AHeroCharacter>();
		TestEqual("Ally ignored", UCombatLibrary::DeliverHit(F.Hero, Hit), ECombatHitResult::Ignored);
		TestFalse("Ally no reward", F.Combat()->HasCounterWindow());
	});
	It("uses the existing recovery counter and consumes its multiplier on the first target only", [this]()
	{
		FPerfectFixture F; TestTrue("Started", F.Start()); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		F.Combat()->RequestAction(EHeroAction::Light);
		F.Combat()->CloseInvulnerableWindow(); F.Combat()->OpenCancelWindow({ EHeroAction::Light, EHeroAction::Heavy, EHeroAction::BlockStart });
		TestEqual("Buffered recovery Light", F.Combat()->GetActionState(), EHeroActionState::LightAttack);
		auto* Trace = F.Hero->GetMeleeTraceComponent();
		TestTrue("Same counter payload", Trace->GetPendingAttackTemplate().bIsParryCounter);
		auto* First = F.World->SpawnActor<ATestDummy>(); First->DispatchBeginPlay();
		auto* Second = F.World->SpawnActor<ATestDummy>(); Second->DispatchBeginPlay();
		Trace->BeginHitWindow(); Trace->TryHitTarget(First); Trace->TryHitTarget(Second);
		TestEqual("Counter first damage", First->GetHealth()->GetCurrentHealth(), 85.f);
		TestEqual("Normal second damage", Second->GetHealth()->GetCurrentHealth(), 90.f);
		TestEqual("Counter consumed", F.Combat()->GetCounterTimeRemaining(), 0.f);
	});
	It("clears reward on hero-clock expiry, interruption, reset and death", [this]()
	{
		FPerfectFixture F; F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		F.Combat()->TickComponent(1.01f, LEVELTICK_All, nullptr); TestFalse("Expired", F.Combat()->HasCounterWindow());
		F.Combat()->ResetToIdle(); F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		F.Combat()->CloseInvulnerableWindow(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		TestFalse("Interrupted counter cleared", F.Combat()->HasCounterWindow());
		F.Combat()->ResetToIdle(); TestEqual("Reset eligibility", F.Combat()->GetPerfectDodgeTimeRemaining(), 0.f);
		F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming()); F.Combat()->HandleOwnerDeath(F.Incoming());
		TestFalse("Death counter cleared", F.Combat()->HasCounterWindow()); TestEqual("Death eligibility", F.Combat()->GetPerfectDodgeTimeRemaining(), 0.f);
	});
	It("rejects a perfect window beyond i-frames before stamina spend", [this]()
	{
		FPerfectFixture F;
		F.Hero->GetHeroClassDefinition()->Dodge.PerfectWindowSeconds = 0.5f;
		FString Error; TestFalse("Invalid timing", F.Hero->GetHeroClassDefinition()->ValidateDodge(EHeroDodgeDirection::Backward, Error));
		AddExpectedError(TEXT("refused: Perfect dodge"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse("Refused", F.Combat()->RequestAction(EHeroAction::Dodge));
		TestEqual("No spend", F.Hero->GetStaminaComponent()->GetCurrentStamina(), 100.f);
	});
	It("external montage interruption clears eligibility and counter without a stale callback", [this]()
	{
		FPerfectFixture F; F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		F.Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
		F.Hero->GetMesh()->TickAnimation(0.01f, false);
		F.Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
		TestFalse("No interrupted reward", F.Combat()->HasCounterWindow());
		TestEqual("No eligibility", F.Combat()->GetPerfectDodgeTimeRemaining(), 0.f);
		TestEqual("Idle restored", F.Combat()->GetActionState(), EHeroActionState::Idle);
	});
	It("saved custom clips contain directional root travel and normalized finite original pose keys", [this]()
	{
		for (const TCHAR* Direction : { TEXT("F"), TEXT("B"), TEXT("L"), TEXT("R") })
		{
			const FString Path = FString::Printf(TEXT("/Game/CastleDefender/Hero/A_Warlord_CustomDodge_%s"), Direction);
			UAnimSequence* Clip = LoadObject<UAnimSequence>(nullptr, *Path);
			if (!TestNotNull(Path, Clip)) { continue; }
			TestEqual("0.6 second clip", Clip->GetPlayLength(), 0.6f, 0.001f);
			TestTrue("Root motion enabled", Clip->bEnableRootMotion);
			const IAnimationDataModel* Model = Clip->GetDataModel();
			TArray<FTransform> RootKeys; Model->GetBoneTrackTransforms(TEXT("root"), RootKeys);
			TestEqual("60 fps endpoint-inclusive keys", RootKeys.Num(), 37);
			if (RootKeys.Num() != 37) { continue; }
			const FVector Travel = RootKeys.Last().GetTranslation() - RootKeys[0].GetTranslation();
			const FVector Expected = FCString::Strcmp(Direction, TEXT("F")) == 0 ? FVector(0, 500, 0)
				: FCString::Strcmp(Direction, TEXT("B")) == 0 ? FVector(0, -500, 0)
				: FCString::Strcmp(Direction, TEXT("L")) == 0 ? FVector(500, 0, 0) : FVector(-500, 0, 0);
			TestTrue("Directional travel", Travel.Equals(Expected, 0.01f));
			TArray<FName> Names; Model->GetBoneTrackNames(Names);
			for (FName Name : Names)
			{
				TArray<FTransform> Keys; Model->GetBoneTrackTransforms(Name, Keys);
				for (const FTransform& Key : Keys)
				{
					TestFalse("Finite key", Key.ContainsNaN()); TestTrue("Unit quaternion", Key.GetRotation().IsNormalized());
				}
			}
			for (const FTransform& Key : RootKeys) { TestTrue("Root never spins capsule", Key.GetRotation().Equals(RootKeys[0].GetRotation(), 0.001f)); }
			TArray<FTransform> PelvisKeys; Model->GetBoneTrackTransforms(TEXT("pelvis"), PelvisKeys);
			TestTrue("Pose returns to idle", PelvisKeys.Last().Equals(PelvisKeys[0], 0.001f));
			TestFalse("Original body action", PelvisKeys[15].Equals(PelvisKeys[0], 0.01f));
		}
	});
	It("saved perfect-dodge feedback has a dedicated sound and translucent skeletal material", [this]()
	{
		UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/CastleDefender/Feedback/DT_Feedback"));
		if (!TestNotNull("Saved table", Table)) { return; }
		const FFeedbackRow* Row = Table->FindRow<FFeedbackRow>(FeedbackTags::Combat_PerfectDodge.GetTag().GetTagName(), TEXT("test"));
		if (!TestNotNull("Saved cue", Row)) { return; }
		TestNotNull("Sound", Row->Sound.Get());
		if (TestNotNull("Ghost material", Row->AfterimageMaterial.Get()))
		{
			TestEqual("Translucent", Row->AfterimageMaterial->GetBlendMode(), BLEND_Translucent);
			TestTrue("Skeletal usage serialized", Row->AfterimageMaterial->GetMaterial()->GetUsageByFlag(MATUSAGE_SkeletalMesh));
		}
		TestEqual("Real seconds", Row->AfterimageSeconds, 0.35f); TestEqual("Opacity", Row->AfterimageOpacity, 0.55f);
	});
	It("saved montage windows preserve startup, invulnerability and recovery across all custom sources", [this]()
	{
		FPerfectFixture F;
		for (EHeroDodgeDirection Direction : { EHeroDodgeDirection::Forward, EHeroDodgeDirection::Backward, EHeroDodgeDirection::Left, EHeroDodgeDirection::Right })
		{
			FCombatActionTiming Timing; FString Error;
			TestTrue("Valid dodge", F.Hero->GetHeroClassDefinition()->ValidateDodge(Direction, Error));
			TestTrue("Inspect timing", FCombatActionTiming::InspectMontage(F.Hero->GetHeroClassDefinition()->Dodge.GetMontage(Direction), Timing));
			TestEqual("Total", Timing.TotalDuration, 0.6f, 0.001f);
			TestEqual("I-frame start", Timing.InvulnerableWindowStart, 0.10f, 0.001f);
			TestEqual("I-frame end", Timing.InvulnerableWindowEnd, 0.35f, 0.001f);
			TestEqual("Recovery start", Timing.CancelWindowStart, 0.40f, 0.001f);
			TestEqual("Recovery end", Timing.CancelWindowEnd, 0.59f, 0.001f);
		}
	});
	It("creates one fixed-world frozen ghost with no collision or ticking, then expires in real time", [this]()
	{
		FPerfectFixture F; auto* Feedback = UFeedbackSubsystem::Get(F.Hero);
		auto* Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_PerfectDodge });
		FFeedbackRow* Row = Table->FindRow<FFeedbackRow>(FeedbackTags::Combat_PerfectDodge.GetTag().GetTagName(), TEXT("test"));
		Row->AfterimageMaterial = UMaterial::GetDefaultMaterial(MD_Surface); Row->AfterimageSeconds = 0.03f;
		Feedback->SetFeedbackTable(Table); F.Start(); UCombatLibrary::DeliverHit(F.Hero, F.Incoming());
		TArray<UPoseableMeshComponent*> Ghosts; F.Hero->GetComponents(Ghosts);
		TestEqual("One frozen pose", Ghosts.Num(), 1); if (Ghosts.IsEmpty()) { return; }
		TestFalse("No tick", Ghosts[0]->IsComponentTickEnabled()); TestEqual("No collision", Ghosts[0]->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		const FVector Location = Ghosts[0]->GetComponentLocation();
		auto* Material = Cast<UMaterialInstanceDynamic>(Ghosts[0]->GetMaterial(0));
		FPlatformProcess::Sleep(0.01f); FTSTicker::GetCoreTicker().Tick(0.01f);
		if (TestNotNull("Dynamic fade material", Material)) { TestTrue("Opacity fades", Material->K2_GetScalarParameterValue(TEXT("GhostOpacity")) < Row->AfterimageOpacity); }
		F.Hero->SetActorLocation(FVector(500.f, 0.f, 0.f));
		TestEqual("Fixed in world", Ghosts[0]->GetComponentLocation(), Location);
		F.Hero->CustomTimeDilation = 0.01f;
		FPlatformProcess::Sleep(0.04f); FTSTicker::GetCoreTicker().Tick(0.04f);
		Ghosts.Reset(); F.Hero->GetComponents(Ghosts); TestEqual("Real-time expiry", Ghosts.Num(), 0);
	});
}
#endif
