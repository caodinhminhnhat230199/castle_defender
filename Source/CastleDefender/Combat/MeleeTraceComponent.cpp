#include "Combat/MeleeTraceComponent.h"

#include "Combat/CombatLibrary.h"
#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Character.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UMeleeTraceComponent::UMeleeTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMeleeTraceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!TraceMeshComponent.IsValid() && GetOwner())
	{
		if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			TraceMeshComponent = Char->GetMesh();
		}
		else
		{
			TraceMeshComponent = GetOwner()->FindComponentByClass<UMeshComponent>();
		}
	}
}

void UMeleeTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsRegistered())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

	if (!bHitWindowActive)
	{
		return;
	}

	const uint32 SweepGeneration = HitWindowGeneration;
	const TArray<FVector> CurrentPositions = ComputeSamplePositions();
	if (PreviousSamplePositions.Num() == CurrentPositions.Num() && CurrentPositions.Num() > 0)
	{
		ProcessSweepStep(PreviousSamplePositions, CurrentPositions);
	}

	// DeliverHit may close or replace this window; never overwrite the new window's starting samples.
	if (SweepGeneration == HitWindowGeneration && bHitWindowActive) { PreviousSamplePositions = CurrentPositions; }
}

void UMeleeTraceComponent::SetPendingAttack(const FCombatHit& Template, float Radius, FName StartSocket, FName EndSocket, int32 SamplePoints)
{
	PendingHitTemplate = Template;
	CounterBaseDamage = Template.Damage;
	TraceRadius = Radius;
	SocketStart = StartSocket;
	SocketEnd = EndSocket;
	NumberOfSamplePoints = FMath::Clamp(SamplePoints, 1, 10);
}

void UMeleeTraceComponent::BeginHitWindow()
{
	++HitWindowGeneration;
	bHitWindowActive = true;
	AlreadyHitActors.Reset();
	PreviousSamplePositions = ComputeSamplePositions();
	SetComponentTickEnabled(true);
	OnHitWindowBegin.Broadcast();
}

void UMeleeTraceComponent::SetParryCounterMultiplier(float Multiplier)
{
	PendingHitTemplate.bIsParryCounter = true;
	PendingHitTemplate.Damage = CounterBaseDamage * Multiplier;
}

void UMeleeTraceComponent::EndHitWindow()
{
	++HitWindowGeneration;
	bHitWindowActive = false;
	SetComponentTickEnabled(false);
	PreviousSamplePositions.Empty();
}

TArray<FVector> UMeleeTraceComponent::ComputeSamplePositions() const
{
	TArray<FVector> Positions;
	FVector StartLoc = FVector::ZeroVector;
	FVector EndLoc = FVector::ZeroVector;
	bool bSocketsValid = false;

	USceneComponent* Mesh = TraceMeshComponent.Get();
	if (!Mesh && GetOwner())
	{
		if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			Mesh = Char->GetMesh();
		}
		else
		{
			Mesh = GetOwner()->FindComponentByClass<UMeshComponent>();
		}
	}

	if (Mesh)
	{
		if (Mesh->DoesSocketExist(SocketStart) && Mesh->DoesSocketExist(SocketEnd))
		{
			StartLoc = Mesh->GetSocketLocation(SocketStart);
			EndLoc = Mesh->GetSocketLocation(SocketEnd);
			bSocketsValid = true;
		}
	}

	if (!bSocketsValid)
	{
		AActor* OwnerActor = GetOwner();
		if (OwnerActor)
		{
			const FVector Forward = OwnerActor->GetActorForwardVector();
			const FVector Origin = OwnerActor->GetActorLocation();
			StartLoc = Origin + Forward * 50.f;
			EndLoc = Origin + Forward * 180.f + FVector(0.f, 0.f, 20.f);
		}
	}

	const int32 Count = FMath::Max(1, NumberOfSamplePoints);
	if (Count == 1)
	{
		Positions.Add(EndLoc);
	}
	else
	{
		for (int32 i = 0; i < Count; ++i)
		{
			const float Alpha = static_cast<float>(i) / static_cast<float>(Count - 1);
			Positions.Add(FMath::Lerp(StartLoc, EndLoc, Alpha));
		}
	}

	return Positions;
}

void UMeleeTraceComponent::ProcessSweepStep(const TArray<FVector>& PreviousPositions, const TArray<FVector>& CurrentPositions)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
	const bool bDebugTrace = (GameDebug::CVarCombatTrace.GetValueOnGameThread() > 0);
	#endif

	FCollisionQueryParams QueryParams(TEXT("MeleeTraceSweep"), false, GetOwner());
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	const uint32 SweepGeneration = HitWindowGeneration;
	const int32 SampleCount = FMath::Min(PreviousPositions.Num(), CurrentPositions.Num());
	for (int32 i = 0; i < SampleCount; ++i)
	{
		if (!bHitWindowActive || SweepGeneration != HitWindowGeneration) { return; }
		const FVector& PrevPos = PreviousPositions[i];
		const FVector& CurrPos = CurrentPositions[i];

		TArray<FHitResult> HitResults;
		World->SweepMultiByObjectType(
			HitResults,
			PrevPos,
			CurrPos,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams
		);

#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
		if (bDebugTrace)
		{
			DrawDebugSphere(World, CurrPos, TraceRadius, 8, FColor::Yellow, false, 0.15f);
			DrawDebugLine(World, PrevPos, CurrPos, FColor::Orange, false, 0.15f);
		}
#endif

		for (const FHitResult& Hit : HitResults)
		{
			TryHitTarget(Hit.GetActor(), FVector(Hit.ImpactPoint), UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()));
			// Parry/Staggered callbacks can clear the aliased sample array synchronously.
			if (!bHitWindowActive || SweepGeneration != HitWindowGeneration) { return; }
		}
	}
}

bool UMeleeTraceComponent::TryHitTarget(AActor* HitActor, const FVector& ImpactPoint, EPhysicalSurface Surface)
{
	if (!bHitWindowActive)
	{
		return false;
	}

	if (!HitActor || HitActor == GetOwner() || AlreadyHitActors.Contains(HitActor))
	{
		return false;
	}

	if (GetOwner() && !AreHostile(GetOwner(), HitActor))
	{
		return false;
	}

	// R-CMB-01, AC-CMB-01: hit each target at most once per swing window
	AlreadyHitActors.Add(HitActor);

	FCombatHit HitToSend = PendingHitTemplate;
	if (HitToSend.bIsParryCounter)
	{
		PendingHitTemplate.bIsParryCounter = false;
		PendingHitTemplate.Damage = CounterBaseDamage;
		OnParryCounterConsumed.Broadcast();
	}
	HitToSend.Instigator = GetOwner();
	HitToSend.Surface = Surface;
	HitToSend.HitLocation = ImpactPoint.IsNearlyZero() ? HitActor->GetActorLocation() : ImpactPoint;
	HitToSend.HitDirection = (HitActor->GetActorLocation() - (GetOwner() ? GetOwner()->GetActorLocation() : HitToSend.HitLocation)).GetSafeNormal();

	const ECombatHitResult Result = UCombatLibrary::DeliverHit(HitActor, HitToSend);
	OnHitResolved.Broadcast(HitActor, Result);

#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
	UWorld* World = GetWorld();
	if (World && GameDebug::CVarCombatTrace.GetValueOnGameThread() > 0)
	{
		FString HitSet;
		for (const TWeakObjectPtr<AActor>& Actor : AlreadyHitActors)
		{
			if (Actor.IsValid())
			{
				HitSet += Actor->GetName() + TEXT(" ");
				DrawDebugSphere(World, Actor->GetActorLocation(), 20.f, 8, FColor::Red, false, 0.15f);
			}
		}
		DrawDebugPoint(World, HitToSend.HitLocation, 12.f, FColor::Red, false, 1.0f);
		DrawDebugString(World, HitToSend.HitLocation,
			FString::Printf(TEXT("Hit: %s [%s] (Total: %d)\nAlready hit: %s"),
				*HitActor->GetName(),
				*UEnum::GetValueAsString(Result),
				AlreadyHitActors.Num(), *HitSet),
			nullptr, FColor::White, 1.0f);
	}
#endif

	return true;
}

