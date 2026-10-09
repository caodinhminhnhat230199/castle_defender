#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyTestFixture.h"
#include "Tests/FeedbackTestListener.h"
#include "Tests/CombatTestListener.h"
#include "Combat/CombatLibrary.h"
#include "Combat/MeleeTraceComponent.h"
#include "Combat/TestDummy.h"
#include "Combat/HealthComponent.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Hero/HeroCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

namespace
{
	struct FHitFeedbackFixture : FEnemyTestWorld
	{
		AHeroCharacter* Hero = World->SpawnActor<AHeroCharacter>();
		ATestDummy* Target = World->SpawnActor<ATestDummy>();
		UFeedbackTestListener* Listener = NewObject<UFeedbackTestListener>();
		FHitFeedbackFixture()
		{
			World->InitializeActorsForPlay(FURL());
			Hero->GetHealthComponent()->InitializeHealth(100.f, 0.f);
			Target->GetHealth()->InitializeHealth(100.f, 0.f);
			UFeedbackSubsystem* Feedback = World->GetSubsystem<UFeedbackSubsystem>();
			Feedback->SetFeedbackTable(UFeedbackTestListener::MakeTable({
				FeedbackTags::Combat_Hit_Light, FeedbackTags::Combat_Hit_Heavy,
				FeedbackTags::Combat_Hit_Light_Armored, FeedbackTags::Combat_Hit_Heavy_Armored,
				FeedbackTags::Combat_Block, FeedbackTags::Combat_BlockBreak, FeedbackTags::Combat_Parry,
				FeedbackTags::Hero_Damaged }));
			Feedback->OnFeedbackPlayed.AddDynamic(Listener, &UFeedbackTestListener::HandlePlayed);
		}
	};
}

BEGIN_DEFINE_SPEC(FHitFeedbackSpec, "CastleDefender.Feedback.HitPipeline", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHitFeedbackSpec)

void FHitFeedbackSpec::Define()
{
	It("plays one armored Heavy outcome with resolved context, including lethal impacts", [this]()
	{
		FHitFeedbackFixture F;
		F.Target->GetHealth()->InitializeHealth(100.f, 0.5f);
		FCombatHit Hit;
		Hit.Instigator = F.Hero;
		Hit.Damage = 20.f;
		Hit.bIsHeavy = true;
		Hit.HitLocation = FVector(123.f, 45.f, 67.f);
		Hit.HitDirection = FVector::ForwardVector;
		TestEqual("Hit delivered", UCombatLibrary::DeliverHit(F.Target, Hit), ECombatHitResult::Hit);
		TestEqual("One outcome", F.Listener->PlayedCount, 1);
		TestEqual("Armored variant", F.Listener->LastPlayed, FeedbackTags::Combat_Hit_Heavy_Armored.GetTag());
		TestTrue("Heavy context", F.Listener->LastContext.bIsHeavy);
		TestTrue("Armor resolved before hit", F.Listener->LastContext.bTargetArmored);
		TestEqual("Exact impact", F.Listener->LastContext.Location, Hit.HitLocation);
		TestEqual("Direction", F.Listener->LastContext.Direction, Hit.HitDirection);
		Hit.Damage = 1000.f;
		TestEqual("Lethal delivered", UCombatLibrary::DeliverHit(F.Target, Hit), ECombatHitResult::Killed);
		TestEqual("Lethal outcome also once", F.Listener->PlayedCount, 2);
		UCombatLibrary::DeliverHit(F.Target, Hit);
		TestEqual("Dead target emits nothing", F.Listener->PlayedCount, 2);
	});
	It("emits Hero.Damaged only after positive HP loss and keeps defensive outcomes exclusive", [this]()
	{
		FHitFeedbackFixture F;
		// Hero already intercepts, so use an ordinary target for defensive exclusivity.
		UMockHitInterceptorComponent* TargetInterceptor = NewObject<UMockHitInterceptorComponent>(F.Target);
		F.Target->AddInstanceComponent(TargetInterceptor);
		FCombatHit Hit;
		Hit.Instigator = F.Hero;
		Hit.Damage = 20.f;
		TargetInterceptor->ResponseResult = ECombatHitResult::Blocked;
		UCombatLibrary::DeliverHit(F.Target, Hit);
		TestTrue("Only Block", F.Listener->PlayedTags == TArray<FGameplayTag>{ FeedbackTags::Combat_Block });
		F.Listener->PlayedTags.Empty(); F.Listener->PlayedCount = 0;
		TargetInterceptor->ResponseResult = ECombatHitResult::Evaded;
		UCombatLibrary::DeliverHit(F.Target, Hit);
		TestEqual("Evade emits nothing", F.Listener->PlayedCount, 0);
		Hit.Instigator = F.Target;
		UCombatLibrary::DeliverHit(F.Hero, Hit);
		TestEqual("Hero outcome plus damage observer event", F.Listener->PlayedCount, 2);
		TestTrue("One Light outcome", F.Listener->PlayedTags.IsValidIndex(0) && F.Listener->PlayedTags[0] == FeedbackTags::Combat_Hit_Light);
		TestEqual("Damage follows outcome", F.Listener->LastPlayed, FeedbackTags::Hero_Damaged.GetTag());
		TestEqual("Magnitude is actual HP loss", F.Listener->LastContext.Magnitude, 20.f);
		Hit.Damage = 0.f;
		UCombatLibrary::DeliverHit(F.Hero, Hit);
		TestEqual("Zero damage adds only the impact event", F.Listener->PlayedCount, 3);
		TestEqual("Zero damage has no damaged event", F.Listener->LastPlayed, FeedbackTags::Combat_Hit_Light.GetTag());
	});
	It("carries the physical surface returned by a real melee sweep", [this]()
	{
		FHitFeedbackFixture F;
		F.Target->SetActorLocation(FVector(300.f, 0.f, 0.f));
		UStaticMeshComponent* Mesh = F.Target->FindComponentByClass<UStaticMeshComponent>();
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		UPhysicalMaterial* Material = NewObject<UPhysicalMaterial>(Mesh);
		Material->SurfaceType = SurfaceType2;
		Mesh->SetPhysMaterialOverride(Material);
		// Chaos query-material cache is synchronized by the physics frame, not SetPhysMaterialOverride.
		F.World->Tick(LEVELTICK_All, 1.f / 60.f);
		TestEqual("Fixture body uses requested PM", Mesh->BodyInstance.GetSimplePhysicalMaterial(), Material);
		TArray<FHitResult> ProbeHits;
		FCollisionQueryParams ProbeParams(TEXT("PhysicalSurfaceProbe"), false, F.Hero);
		ProbeParams.bReturnPhysicalMaterial = true;
		F.World->SweepMultiByObjectType(ProbeHits, FVector(150.f, 0.f, 0.f), FVector(400.f, 0.f, 0.f), FQuat::Identity,
			FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeSphere(25.f), ProbeParams);
		for (const FHitResult& Probe : ProbeHits)
		{
			AddInfo(FString::Printf(TEXT("Raw sweep: %s PM=%s surface=%d"), *GetNameSafe(Probe.GetComponent()),
				*GetNameSafe(Probe.PhysMaterial.Get()), static_cast<int32>(UPhysicalMaterial::DetermineSurfaceType(Probe.PhysMaterial.Get()))));
		}
		UMeleeTraceComponent* Trace = NewObject<UMeleeTraceComponent>(F.Hero);
		F.Hero->AddInstanceComponent(Trace);
		Trace->RegisterComponent();
		FCombatHit Hit;
		Hit.Damage = 10.f;
		Trace->SetPendingAttack(Hit);
		Trace->BeginHitWindow();
		Trace->ProcessSweepStep({ FVector(150.f, 0.f, 0.f) }, { FVector(400.f, 0.f, 0.f) });
		TestEqual("Physical sweep hits once", F.Listener->PlayedCount, 1);
		TestEqual("Armor PM reaches feedback", static_cast<int32>(F.Listener->LastContext.Surface.GetValue()), static_cast<int32>(SurfaceType2));
	});
}
#endif
