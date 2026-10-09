#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Hero/LockOnComponent.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/TestDummy.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/BlendSpace.h"

namespace
{
	struct FLockFixture : FHeroCombatFixture
	{
		FLockFixture()
		{
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			BeginPlay();
		}
		~FLockFixture()
		{
			Hero->GetLockOnComponent()->Release();
			GEngine->DestroyWorldContext(World);
		}
		ATestDummy* Dummy(const FVector& Location)
		{
			ATestDummy* Result = World->SpawnActor<ATestDummy>(Location, FRotator::ZeroRotator);
			Result->DispatchBeginPlay();
			return Result;
		}
		void Wall()
		{
			AActor* Actor = World->SpawnActor<AActor>();
			UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
			Actor->SetRootComponent(Box);
			Actor->AddInstanceComponent(Box);
			Box->SetMobility(EComponentMobility::Movable);
			Box->SetBoxExtent(FVector(25.f, 100.f, 200.f));
			Box->SetCollisionProfileName(TEXT("BlockAll"));
			Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Box->SetWorldLocation(FVector(250.f, 0.f, 0.f));
			Box->RegisterComponent();
		}
	};
}

BEGIN_DEFINE_SPEC(FLockOnSpec, "CastleDefender.Combat.Hero.LockOn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FLockOnSpec)

void FLockOnSpec::Define()
{
	It("owns lock-on on the hero pawn", [this]()
	{
		FHeroCombatFixture Fixture;
		UClass* LockClass = FindObject<UClass>(nullptr, TEXT("/Script/CastleDefender.LockOnComponent"));
		TestNotNull("Lock-on class exists", LockClass);
		if (LockClass) { TestNotNull("Hero owns lock-on", Fixture.Hero->FindComponentByClass(LockClass)); }
	});
	It("acquires the visible screen-centre hostile, then distance breaks equal-angle ties", [this]()
	{
		FLockFixture F;
		F.Dummy(FVector(450.f, -200.f, 0.f));
		ATestDummy* Centre = F.Dummy(FVector(500.f, 0.f, 0.f));
		F.Dummy(FVector(450.f, 200.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		TestEqual("Centre", Lock->GetLockOnTarget(), static_cast<AActor*>(Centre));
		TestTrue("Camera tick active", Lock->IsComponentTickEnabled());
		Lock->Toggle();
		TestNull("Toggle releases", Lock->GetLockOnTarget());
		TestFalse("Idle does no camera work", Lock->IsComponentTickEnabled());
		ATestDummy* Near = F.Dummy(FVector(300.f, 0.f, 0.f));
		Lock->Toggle();
		TestEqual("Nearest at same angle", Lock->GetLockOnTarget(), static_cast<AActor*>(Near));
	});
	It("never acquires allies, dead targets, targets behind camera, beyond range or behind a wall", [this]()
	{
		FLockFixture F;
		F.Dummy(FVector(1600.f, 0.f, 0.f));
		F.Dummy(FVector(-800.f, 0.f, 0.f));
		ATestDummy* Ally = F.Dummy(FVector(500.f, -200.f, 0.f));
		Ally->SetGenericTeamId(F.Hero->GetGenericTeamId());
		ATestDummy* Dead = F.Dummy(FVector(500.f, 200.f, 0.f));
		Dead->ApplyDebugHit(1000.f);
		F.Dummy(FVector(500.f, 0.f, 0.f));
		F.Wall();
		F.Hero->GetLockOnComponent()->Toggle();
		TestNull("No eligible candidate", F.Hero->GetLockOnComponent()->GetLockOnTarget());
	});
	It("switches to the nearest screen neighbour on each side without wrapping at the edge", [this]()
	{
		FLockFixture F;
		ATestDummy* Left = F.Dummy(FVector(500.f, -250.f, 0.f));
		ATestDummy* Centre = F.Dummy(FVector(500.f, 0.f, 0.f));
		ATestDummy* Right = F.Dummy(FVector(500.f, 250.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		Lock->Switch(1.f);
		TestEqual("Right", Lock->GetLockOnTarget(), static_cast<AActor*>(Right));
		Lock->Switch(1.f);
		TestEqual("Edge holds", Lock->GetLockOnTarget(), static_cast<AActor*>(Right));
		Lock->Switch(-1.f);
		TestEqual("Centre again", Lock->GetLockOnTarget(), static_cast<AActor*>(Centre));
		Lock->Switch(-1.f);
		TestEqual("Left", Lock->GetLockOnTarget(), static_cast<AActor*>(Left));
	});
	It("holds inside break distance but releases outside it and when affiliation changes", [this]()
	{
		FLockFixture F;
		ATestDummy* Dummy = F.Dummy(FVector(500.f, 0.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		Dummy->SetActorLocation(FVector(1700.f, 0.f, 0.f));
		Lock->ValidateTarget();
		TestEqual("Acquire range is not break distance", Lock->GetLockOnTarget(), static_cast<AActor*>(Dummy));
		Dummy->SetActorLocation(FVector(2100.f, 0.f, 0.f));
		Lock->ValidateTarget();
		TestNull("Distance release", Lock->GetLockOnTarget());
		Dummy->SetActorLocation(FVector(500.f, 0.f, 0.f));
		Lock->Toggle();
		Dummy->SetGenericTeamId(F.Hero->GetGenericTeamId());
		Lock->ValidateTarget();
		TestNull("New ally released", Lock->GetLockOnTarget());
	});
	It("retargets immediately on death or destruction and releases if none remain", [this]()
	{
		FLockFixture F;
		ATestDummy* Centre = F.Dummy(FVector(500.f, 0.f, 0.f));
		ATestDummy* Next = F.Dummy(FVector(400.f, 200.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		Centre->ApplyDebugHit(1000.f);
		TestEqual("Next hostile", Lock->GetLockOnTarget(), static_cast<AActor*>(Next));
		Next->Destroy();
		TestNull("None remain", Lock->GetLockOnTarget());
		TestFalse("Destroy stops camera work", Lock->IsComponentTickEnabled());
	});
	It("LOS grace uses world time despite per-hero hit stop", [this]()
	{
		FLockFixture F;
		ATestDummy* Dummy = F.Dummy(FVector(500.f, 0.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		F.Wall();
		F.Hero->CustomTimeDilation = 0.01f;
		Lock->ValidateTarget();
		// UWorld clamps a single large delta. Advance realistic frames to test an actual 1 s world deadline.
		for (int32 Frame = 0; Frame < 9; ++Frame) { F.World->Tick(LEVELTICK_TimeOnly, 0.1f); }
		Lock->ValidateTarget();
		TestEqual("Brief occlusion holds", Lock->GetLockOnTarget(), static_cast<AActor*>(Dummy));
		F.World->Tick(LEVELTICK_TimeOnly, 0.2f);
		TestTrue("Fixture advanced the world deadline", F.World->GetTimeSeconds() >= 1.f);
		Lock->ValidateTarget();
		TestNull("World grace elapsed", Lock->GetLockOnTarget());
	});
	It("release restores locomotion and hero death cannot reacquire", [this]()
	{
		FLockFixture F;
		F.Hero->GetHeroClassDefinition()->Movement.bFaceCameraDirection = false;
		F.Hero->UpdateFacingPolicy();
		F.Dummy(FVector(500.f, 0.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		Lock->Toggle();
		TestTrue("Strafe facing", F.Hero->GetCharacterMovement()->bUseControllerDesiredRotation);
		TestFalse("No turn-to-movement", F.Hero->GetCharacterMovement()->bOrientRotationToMovement);
		Lock->Release();
		TestTrue("Free movement restored", F.Hero->GetCharacterMovement()->bOrientRotationToMovement);
		Lock->Toggle();
		FCombatHit Hit; Hit.Damage = 1000.f;
		UCombatLibrary::DeliverHit(F.Hero, Hit);
		TestNull("Death releases", Lock->GetLockOnTarget());
		Lock->Toggle();
		TestNull("Dead hero cannot acquire", Lock->GetLockOnTarget());
	});
	It("prefers an eligible locked attack target but never bypasses assist limits or montage facing", [this]()
	{
		FLockFixture F;
		ATestDummy* Centre = F.Dummy(FVector(250.f, 0.f, 0.f));
		ATestDummy* Right = F.Dummy(FVector(250.f, 100.f, 0.f));
		auto* Lock = F.Hero->GetLockOnComponent();
		auto* Combat = F.Hero->GetCombatComponent();
		Lock->Toggle(); Lock->Switch(1.f);
		TestTrue("Heavy starts", Combat->RequestAction(EHeroAction::Heavy));
		TestFalse("Controller cannot turn the attacking body", F.Hero->GetCharacterMovement()->bUseControllerDesiredRotation);
		Combat->OpenRotationAssistWindow();
		TestEqual("Locked preference", Combat->GetAssistTarget(), static_cast<AActor*>(Right));
		Right->SetActorLocation(FVector(800.f, 300.f, 0.f));
		Combat->OpenRotationAssistWindow();
		TestEqual("Out-of-bounds lock falls back", Combat->GetAssistTarget(), static_cast<AActor*>(Centre));
		Lock->Release();
		TestFalse("Release does not rotate a committed attack", F.Hero->GetCharacterMovement()->bUseControllerDesiredRotation);
	});
	It("rejects invalid lock-on tuning at runtime", [this]()
	{
		FLockFixture F;
		F.Dummy(FVector(500.f, 0.f, 0.f));
		F.Hero->GetHeroClassDefinition()->LockOn.BreakDistance = 100.f;
		TestFalse("Ordered ranges", F.Hero->GetHeroClassDefinition()->LockOn.IsValid());
		F.Hero->GetLockOnComponent()->Toggle();
		TestNull("Invalid tuning refuses lock", F.Hero->GetLockOnComponent()->GetLockOnTarget());
	});
	It("saved strafe content evaluates idle and directional poses instead of the reference pose", [this]()
	{
		UBlendSpace* Blend = LoadObject<UBlendSpace>(nullptr, TEXT("/Game/CastleDefender/Hero/BS_Warlord_Strafe"));
		TestNotNull("Saved strafe blendspace", Blend);
		if (!Blend) { return; }
		TArray<FBlendSampleData> Samples;
		int32 CachedIndex = INDEX_NONE;
		TestTrue("Idle has blend samples", Blend->GetSamplesFromBlendInput(FVector::ZeroVector, Samples, CachedIndex, true));
		TestTrue("Idle produces a pose", !Samples.IsEmpty());
		Samples.Reset(); CachedIndex = INDEX_NONE;
		TestTrue("Right strafe has blend samples", Blend->GetSamplesFromBlendInput(FVector(450.f, 0.f, 0.f), Samples, CachedIndex, true));
		TestTrue("Right strafe produces a pose", !Samples.IsEmpty());
	});
}
#endif
