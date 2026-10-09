#include "Combat/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bInitialized)
	{
		InitializeHealth(MaxHealth, BaseArmor);
	}
}

void UHealthComponent::InitializeHealth(float InMaxHealth, float InBaseArmor)
{
	MaxHealth = FMath::Max(InMaxHealth, 1.f);
	BaseArmor = FMath::Clamp(InBaseArmor, 0.f, 0.9f);
	CurrentHealth = MaxHealth;
	bDead = false;
	bInitialized = true;
}

float UHealthComponent::ApplyHit(const FCombatHit& Hit)
{
	const AActor* Owner = GetOwner();
	if (bDead || Hit.Damage <= 0.f || (Owner && !Owner->CanBeDamaged()))
	{
		return 0.f;
	}

	const float Applied = FMath::Min(Hit.Damage, CurrentHealth);
	CurrentHealth -= Applied;
	// Commit this hit's transition before observers can deliver reentrant hits.
	const bool bKilledByThisHit = CurrentHealth <= 0.f;
	if (bKilledByThisHit)
	{
		bDead = true;
	}
	OnDamaged.Broadcast(Hit, CurrentHealth);

	if (bKilledByThisHit)
	{
		OnDeath.Broadcast(Hit);
	}
	return Applied;
}

float UHealthComponent::Heal(float Amount)
{
	if (bDead || Amount <= 0.f)
	{
		return 0.f;
	}

	const float PrevHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	const float ActualHealed = CurrentHealth - PrevHealth;
	if (ActualHealed > 0.f)
	{
		OnHealed.Broadcast(ActualHealed, CurrentHealth);
	}
	return ActualHealed;
}
