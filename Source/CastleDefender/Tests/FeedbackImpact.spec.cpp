#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyTestFixture.h"
#include "Tests/FeedbackTestListener.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/StaminaComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/Ticker.h"
#include "HAL/PlatformTime.h"
#include "Tests/ImpactTestCamera.h"
#include "Core/GameTuningSettings.h"
#include "GameFramework/PlayerController.h"

namespace
{
	struct FImpactFixture : FEnemyTestWorld
	{
		AHeroCharacter* Hero = World->SpawnActor<AHeroCharacter>();
		AActor* Target = World->SpawnActor<AActor>();
		AActor* Other = World->SpawnActor<AActor>();
		UFeedbackSubsystem* Feedback = World->GetSubsystem<UFeedbackSubsystem>();
		FImpactFixture(float Duration = 0.08f)
		{
			UDataTable* Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Hit_Heavy });
			FFeedbackRow* Row = Table->FindRow<FFeedbackRow>(FeedbackTags::Combat_Hit_Heavy.GetTag().GetTagName(), TEXT("Fixture"));
			Row->HitStopSeconds = Duration;
			Row->bHeroOnly = true;
			Row->BurstLimit = 100;
			Feedback->SetFeedbackTable(Table);
		}
		void Play()
		{
			FFeedbackEventContext Context;
			Context.Instigator = Hero;
			Context.Target = Target;
			Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Context);
		}
	};
}

BEGIN_DEFINE_SPEC(FFeedbackImpactSpec, "CastleDefender.Feedback.Impact", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	TUniquePtr<FImpactFixture> Fixture;
END_DEFINE_SPEC(FFeedbackImpactSpec)

void FFeedbackImpactSpec::Define()
{
	AfterEach([this]() { Fixture.Reset(); });
	It("never stops minor hits or fights without the Hero even when rows request it", [this]()
	{
		Fixture = MakeUnique<FImpactFixture>();
		UDataTable* Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Hit_Light, FeedbackTags::Combat_Hit_Heavy });
		for (auto& Pair : Table->GetRowMap())
		{
			FFeedbackRow* Row = reinterpret_cast<FFeedbackRow*>(Pair.Value);
			Row->HitStopSeconds = 0.1f;
			Row->bHeroOnly = false;
		}
		Fixture->Feedback->SetFeedbackTable(Table);
		FFeedbackEventContext Context;
		Context.Instigator = Fixture->Hero;
		Context.Target = Fixture->Target;
		Fixture->Feedback->Play(FeedbackTags::Combat_Hit_Light, Context);
		TestEqual("Light leaves Hero moving", Fixture->Hero->CustomTimeDilation, 1.f);
		TestEqual("Light leaves target moving", Fixture->Target->CustomTimeDilation, 1.f);
		Context.Instigator = Fixture->Other;
		Fixture->Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Context);
		TestEqual("Non-Hero attacker moving", Fixture->Other->CustomTimeDilation, 1.f);
		TestEqual("Non-Hero target moving", Fixture->Target->CustomTimeDilation, 1.f);
	});
	It("slows only the hit actors and never writes global dilation", [this]()
	{
		Fixture = MakeUnique<FImpactFixture>();
		Fixture->World->GetWorldSettings()->SetTimeDilation(0.25f);
		Fixture->Play();
		TestEqual("Hero stopped", Fixture->Hero->CustomTimeDilation, 0.05f);
		TestEqual("Target stopped", Fixture->Target->CustomTimeDilation, 0.05f);
		TestEqual("Bystander unchanged", Fixture->Other->CustomTimeDilation, 1.f);
		TestEqual("Global unchanged", Fixture->World->GetWorldSettings()->GetEffectiveTimeDilation(), 0.25f);
	});
	LatentIt("restores original actor dilation in real time under global slow motion", [this](const FDoneDelegate& Done)
	{
		Fixture = MakeUnique<FImpactFixture>();
		Fixture->Hero->CustomTimeDilation = 0.8f;
		Fixture->World->GetWorldSettings()->SetTimeDilation(0.25f);
		Fixture->Play();
		TestEqual("Stopped immediately", Fixture->Hero->CustomTimeDilation, 0.05f);
		const double Started = FPlatformTime::Seconds();
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Done, Started](float)
		{
			if (FPlatformTime::Seconds()-Started < 0.16) { return true; }
			TestEqual("Hero original restored", Fixture->Hero->CustomTimeDilation, 0.8f);
			TestEqual("Target restored", Fixture->Target->CustomTimeDilation, 1.f);
			TestEqual("Global still slow", Fixture->World->GetWorldSettings()->GetEffectiveTimeDilation(), 0.25f);
			Done.Execute();
			return false;
		}));
	});
	LatentIt("extends an overlap, caps the row duration and tolerates target destruction", [this](const FDoneDelegate& Done)
	{
		Fixture = MakeUnique<FImpactFixture>(10.f); // Global cap is 0.15 real seconds.
		Fixture->Play();
		const double Started = FPlatformTime::Seconds();
		const TSharedRef<bool> Extended = MakeShared<bool>(false);
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Done, Started, Extended](float)
		{
			const double Age = FPlatformTime::Seconds()-Started;
			if (!*Extended && Age >= 0.07)
			{
				Fixture->Play();
				Fixture->Target->Destroy();
				*Extended = true;
			}
			if (Age < 0.18) { return true; }
			if (Age < 0.22)
			{
				TestEqual("Overlap still held", Fixture->Hero->CustomTimeDilation, 0.05f);
				return true;
			}
			if (Age < 0.30) { return true; }
			TestEqual("Duration capped and restored", Fixture->Hero->CustomTimeDilation, 1.f);
			Done.Execute();
			return false;
		}));
	});
	It("restores pending actors when feedback deinitializes", [this]()
	{
		Fixture = MakeUnique<FImpactFixture>();
		Fixture->Hero->CustomTimeDilation = 0.6f;
		Fixture->Play();
		Fixture->Feedback->Deinitialize();
		TestEqual("Teardown restores original", Fixture->Hero->CustomTimeDilation, 0.6f);
	});
	It("scales direct Hero shakes and replaces only the same tag", [this]()
	{
		Fixture = MakeUnique<FImpactFixture>();
		APlayerController* PC = Fixture->World->SpawnActor<APlayerController>();
		PC->SetAsLocalPlayerController(); // Isolated world has no viewport/ULocalPlayer.
		Fixture->World->AddController(PC); // This fixture does not initialize actors for gameplay.
		TestEqual("Fixture local camera controller registered", Fixture->World->GetFirstPlayerController(), PC);
		AImpactTestCamera* Camera = Fixture->World->SpawnActor<AImpactTestCamera>();
		PC->PlayerCameraManager = Camera;
		Camera->InitializeFor(PC);
		PC->Possess(Fixture->Hero);
		UDataTable* Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Hit_Heavy, FeedbackTags::Combat_Parry });
		for (auto& Pair : Table->GetRowMap())
		{
			FFeedbackRow* Row = reinterpret_cast<FFeedbackRow*>(Pair.Value);
			Row->CameraShake = UImpactTestShake::StaticClass();
			Row->ShakeScale = 0.4f;
		}
		Fixture->Feedback->SetFeedbackTable(Table);
		UGameTuningSettings* Settings = GetMutableDefault<UGameTuningSettings>();
		const float OriginalScale = Settings->CameraShakeScale;
		Settings->CameraShakeScale = 0.5f;
		Fixture->Play();
		TestEqual("Row times global scale", Camera->LastScale, 0.2f);
		const TWeakObjectPtr<UCameraShakeBase> First = Camera->LastStarted;
		FFeedbackEventContext Context;
		Context.Instigator = Fixture->Hero;
		Context.Target = Fixture->Target;
		Fixture->Feedback->Play(FeedbackTags::Combat_Parry, Context);
		TestEqual("Different tag is independent", Camera->Stops, 0);
		Fixture->Play();
		TestEqual("Same tag replaced", Camera->Stops, 1);
		TestTrue("Exact previous instance stopped", Camera->LastStopped == First);
		Settings->CameraShakeScale = 0.f;
		Fixture->Play();
		TestEqual("Zero scale starts no new shake", Camera->Starts, 3);
		TestEqual("Previous same-tag shake removed at zero scale", Camera->Stops, 2);
		Fixture->Feedback->Deinitialize();
		TestEqual("Teardown also stops other tag", Camera->Stops, 3);
		Settings->CameraShakeScale = OriginalScale;
	});
	It("routes world shakes by radius and rejects non-Hero direct shakes", [this]()
	{
		Fixture = MakeUnique<FImpactFixture>();
		APlayerController* PC = Fixture->World->SpawnActor<APlayerController>();
		PC->SetAsLocalPlayerController();
		Fixture->World->AddController(PC);
		AImpactTestCamera* Camera = Fixture->World->SpawnActor<AImpactTestCamera>();
		PC->PlayerCameraManager = Camera;
		Camera->InitializeFor(PC);
		UDataTable* Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Hit_Heavy });
		FFeedbackRow* Row = Table->FindRow<FFeedbackRow>(FeedbackTags::Combat_Hit_Heavy.GetTag().GetTagName(), TEXT("Fixture"));
		Row->CameraShake = UImpactTestShake::StaticClass();
		Fixture->Feedback->SetFeedbackTable(Table);
		FFeedbackEventContext Context;
		Context.Instigator = Fixture->Other;
		Context.Target = Fixture->Target;
		Context.Location = FVector(200.f, 0.f, 0.f);
		Fixture->Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Context);
		TestEqual("Direct non-Hero shake rejected", Camera->Starts, 0);
		Row->ShakeInnerRadius = 100.f;
		Row->ShakeOuterRadius = 300.f;
		Fixture->Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Context);
		TestEqual("Nearby world event started", Camera->Starts, 1);
		TestEqual("Linear radial attenuation", Camera->LastScale, 0.5f);
		Context.Location = FVector(400.f, 0.f, 0.f);
		Fixture->Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Context);
		TestEqual("Outside radius starts nothing", Camera->Starts, 1);
	});
	LatentIt("preserves buffered action across a Heavy hit stop and executes after it", [this](const FDoneDelegate& Done)
	{
		Fixture = MakeUnique<FImpactFixture>(0.08f);
		Fixture->Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(
			LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Fixture->Hero));
		Fixture->Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
		Fixture->Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
		Fixture->Hero->GetStaminaComponent()->InitializeFromConfig(Fixture->Hero->GetHeroClassDefinition()->Stamina);
		UHeroCombatComponent* Combat = Fixture->Hero->GetCombatComponent();
		TestTrue("Heavy starts", Combat->RequestAction(EHeroAction::Heavy));
		Fixture->Play();
		TestEqual("Hero in hit stop", Fixture->Hero->CustomTimeDilation, 0.05f);
		TestFalse("Light cannot start while in Heavy swing", Combat->RequestAction(EHeroAction::Light));
		TestTrue("Light buffered during hit stop", Combat->HasBufferedInput());
		const double Started = FPlatformTime::Seconds();
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Done, Started, Combat](float)
		{
			if (FPlatformTime::Seconds() - Started < 0.12) { return true; }
			TestEqual("Hit stop restored", Fixture->Hero->CustomTimeDilation, 1.f);
			TestTrue("Buffer survives hit stop", Combat->HasBufferedInput());
			Done.Execute();
			return false;
		}));
	});
}
#endif
