#include "Hero/LockOnComponent.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

ULockOnComponent::ULockOnComponent()
{
	// Tick only while locked: interpolate the camera; candidate gathering stays event-driven.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	if (const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner()))
	{
		Hero->GetHealthComponent()->OnDeath.AddDynamic(this, &ULockOnComponent::HandleOwnerDeath);
	}
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Release();
	if (const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner()))
	{
		Hero->GetHealthComponent()->OnDeath.RemoveDynamic(this, &ULockOnComponent::HandleOwnerDeath);
	}
	Super::EndPlay(EndPlayReason);
}

const FHeroLockOnData* ULockOnComponent::GetData() const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero && Hero->GetHeroClassDefinition() ? &Hero->GetHeroClassDefinition()->LockOn : nullptr;
}

bool ULockOnComponent::CanLock() const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero && !Hero->GetHealthComponent()->IsDead() && GetData() && GetData()->IsValid();
}

bool ULockOnComponent::IsAliveHostile(AActor* Candidate) const
{
	if (!IsValid(Candidate) || Candidate->IsActorBeingDestroyed() || !AreHostile(GetOwner(), Candidate)) { return false; }
	const UHealthComponent* Health = Candidate->FindComponentByClass<UHealthComponent>();
	return Health && !Health->IsDead();
}

FVector ULockOnComponent::TargetLocation(AActor* Candidate) const
{
	if (!IsValid(Candidate)) { return FVector::ZeroVector; }
	if (const USkeletalMeshComponent* Mesh = Candidate->FindComponentByClass<USkeletalMeshComponent>();
		Mesh && GetData() && Mesh->GetSkeletalMeshAsset() && Mesh->DoesSocketExist(GetData()->TargetSocket))
	{
		return Mesh->GetSocketLocation(GetData()->TargetSocket);
	}
	if (const UCapsuleComponent* Capsule = Candidate->FindComponentByClass<UCapsuleComponent>()) { return Capsule->GetComponentLocation(); }
	return Candidate->GetActorLocation(); // Sandbox debug dummies have no capsule.
}

FVector ULockOnComponent::GetTargetLocation() const { return TargetLocation(Target.Get()); }

void ULockOnComponent::GetView(FVector& Location, FRotator& Rotation) const
{
	const AHeroCharacter* Hero = CastChecked<AHeroCharacter>(GetOwner());
	if (const APlayerController* PC = Cast<APlayerController>(Hero->GetController())) { PC->GetPlayerViewPoint(Location, Rotation); }
	else { Location = Hero->GetFollowCamera()->GetComponentLocation(); Rotation = Hero->GetFollowCamera()->GetComponentRotation(); }
}

bool ULockOnComponent::HasLOS(AActor* Candidate) const
{
	FVector View; FRotator Rotation;
	GetView(View, Rotation);
	FHitResult Hit;
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, View, TargetLocation(Candidate), ECC_Visibility,
		FCollisionQueryParams(SCENE_QUERY_STAT(LockOnLOS), false, GetOwner()));
	return !bBlocked || Hit.GetActor() == Candidate;
}

TArray<AActor*> ULockOnComponent::GatherCandidates() const
{
	TArray<AActor*> Candidates;
	if (!CanLock()) { return Candidates; }
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic); // Existing sandbox ATestDummy targets use this channel.
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetOwner()->GetActorLocation(), FQuat::Identity, Objects,
		FCollisionShape::MakeSphere(GetData()->Range), FCollisionQueryParams(SCENE_QUERY_STAT(LockOnCandidates), false, GetOwner()));
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (IsAliveHostile(Candidate) && FVector::DistSquared(Candidate->GetActorLocation(), GetOwner()->GetActorLocation()) <= FMath::Square(GetData()->Range)
			&& HasLOS(Candidate)) { Candidates.AddUnique(Candidate); }
	}
	return Candidates;
}

AActor* ULockOnComponent::FindBest(bool bNearest) const
{
	FVector View; FRotator Rotation;
	GetView(View, Rotation);
	AActor* Best = nullptr;
	double BestScore = TNumericLimits<double>::Max(), BestDistance = TNumericLimits<double>::Max();
	for (AActor* Candidate : GatherCandidates())
	{
		const FVector Offset = TargetLocation(Candidate) - View;
		const double Distance = FVector::DistSquared(GetOwner()->GetActorLocation(), Candidate->GetActorLocation());
		const double Dot = FVector::DotProduct(Rotation.Vector(), Offset.GetSafeNormal());
		if (!bNearest && Dot <= 0.0) { continue; }
		const double Score = bNearest ? Distance : 1.0 - Dot;
		if (Score < BestScore || (FMath::IsNearlyEqual(Score, BestScore) && Distance < BestDistance))
		{
			Best = Candidate; BestScore = Score; BestDistance = Distance;
		}
	}
	return Best;
}

bool ULockOnComponent::ProjectCandidate(AActor* Candidate, FVector2D& Screen) const
{
	const AHeroCharacter* Hero = CastChecked<AHeroCharacter>(GetOwner());
	if (const APlayerController* PC = Cast<APlayerController>(Hero->GetController()); PC && PC->GetLocalPlayer())
	{
		return PC->ProjectWorldLocationToScreen(TargetLocation(Candidate), Screen, true);
	}
	// Perspective coordinates for a headless test world with no viewport.
	FVector View; FRotator Rotation;
	GetView(View, Rotation);
	const FVector Local = Rotation.UnrotateVector(TargetLocation(Candidate) - View);
	if (Local.X <= UE_SMALL_NUMBER) { return false; }
	Screen = FVector2D(Local.Y / Local.X, -Local.Z / Local.X);
	return true;
}

void ULockOnComponent::Toggle()
{
	if (Target.IsValid()) { Release(); }
	else if (CanLock()) { SetTarget(FindBest(false)); }
}

void ULockOnComponent::Switch(float Direction)
{
	if (!CanLock() || !Target.IsValid() || !FMath::IsFinite(Direction) || Direction == 0.f) { return; }
	FVector2D Current;
	if (!ProjectCandidate(Target.Get(), Current)) { return; }
	AActor* Best = nullptr;
	double BestDistance = TNumericLimits<double>::Max();
	for (AActor* Candidate : GatherCandidates())
	{
		FVector2D Screen;
		if (Candidate == Target.Get() || !ProjectCandidate(Candidate, Screen) || (Screen.X - Current.X) * Direction <= 0.0) { continue; }
		const double Distance = FVector2D::DistSquared(Screen, Current);
		if (Distance < BestDistance) { Best = Candidate; BestDistance = Distance; }
	}
	if (Best) { SetTarget(Best); }
}

void ULockOnComponent::SetTarget(AActor* NewTarget)
{
	if ((NewTarget && Target.Get() == NewTarget) || (!NewTarget && Target.IsExplicitlyNull())) { return; }
	if (AActor* Old = Target.Get())
	{
		if (UHealthComponent* Health = Old->FindComponentByClass<UHealthComponent>()) { Health->OnDeath.RemoveDynamic(this, &ULockOnComponent::HandleTargetDeath); }
		Old->OnDestroyed.RemoveDynamic(this, &ULockOnComponent::HandleTargetDestroyed);
	}
	GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
	Target = NewTarget;
	LOSLostAt = -1.0;
	if (NewTarget)
	{
		NewTarget->FindComponentByClass<UHealthComponent>()->OnDeath.AddDynamic(this, &ULockOnComponent::HandleTargetDeath);
		NewTarget->OnDestroyed.AddDynamic(this, &ULockOnComponent::HandleTargetDestroyed);
		GetWorld()->GetTimerManager().SetTimer(ValidationTimer, this, &ULockOnComponent::ValidateTarget, GetData()->ValidationInterval, true);
	}
	SetComponentTickEnabled(NewTarget != nullptr);
	CastChecked<AHeroCharacter>(GetOwner())->UpdateFacingPolicy();
	OnLockOnTargetChanged.Broadcast(NewTarget);
}

void ULockOnComponent::Release()
{
	SetTarget(nullptr);
	if (GetWorld()) { GetWorld()->GetTimerManager().ClearTimer(ValidationTimer); }
	SetComponentTickEnabled(false);
}

void ULockOnComponent::Retarget() { SetTarget(CanLock() ? FindBest(true) : nullptr); }
void ULockOnComponent::HandleTargetDeath(const FCombatHit& Hit) { Retarget(); }
void ULockOnComponent::HandleTargetDestroyed(AActor* DestroyedActor) { Retarget(); }
void ULockOnComponent::HandleOwnerDeath(const FCombatHit& Hit) { Release(); }

void ULockOnComponent::ValidateTarget()
{
	if (!CanLock()) { Release(); return; }
	if (!IsAliveHostile(Target.Get())) { Retarget(); return; }
	if (FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetActorLocation()) > FMath::Square(GetData()->BreakDistance)) { Release(); return; }
	const double Now = GetWorld()->GetTimeSeconds(); // D-20: LOS grace follows world game time, never hero hit stop.
	if (HasLOS(Target.Get())) { LOSLostAt = -1.0; }
	else
	{
		if (LOSLostAt < 0.0) { LOSLostAt = Now; }
		if (Now - LOSLostAt >= GetData()->LOSGraceTime) { Release(); }
	}
}

void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	if (!CanLock() || !Target.IsValid() || !Hero->GetController()) { return; }
	const FVector Camera = Hero->GetFollowCamera()->GetComponentLocation();
	FRotator Desired = (GetTargetLocation() - Camera).Rotation();
	Desired.Pitch = FMath::Clamp(Desired.Pitch, GetData()->MinPitch, GetData()->MaxPitch);
	Desired.Roll = 0.f;
	Hero->GetController()->SetControlRotation(FMath::RInterpTo(Hero->GetController()->GetControlRotation(), Desired, DeltaTime, GetData()->CameraInterpSpeed));
	// Facing stays under CharacterMovement in locomotion; authored assist alone turns an attacking hero.
}
