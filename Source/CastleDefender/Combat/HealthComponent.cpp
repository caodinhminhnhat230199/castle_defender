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
	OnDamaged.Broadcast(Hit, CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		bDead = true;
		OnDeath.Broadcast(Hit);
	}
	return Applied;
}
