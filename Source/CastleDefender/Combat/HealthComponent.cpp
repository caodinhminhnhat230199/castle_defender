#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"

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

float UHealthComponent::ComputeDamageAfterArmor(float Damage, float Armor, bool bArmorBroken, float ArmorBrokenMultiplier)
{
	const float EffectiveArmor = FMath::Clamp(Armor, 0.f, 0.9f)
		* (bArmorBroken ? FMath::Clamp(ArmorBrokenMultiplier, 0.f, 1.f) : 1.f);
	return FMath::Max(0.f, Damage) * (1.f - EffectiveArmor);
}

float UHealthComponent::ApplyHit(const FCombatHit& Hit)
{
	const AActor* Owner = GetOwner();
	if (bDead || Hit.Damage <= 0.f || (Owner && !Owner->CanBeDamaged()))
	{
		return 0.f;
	}

	const UCombatStateComponent* States = Owner ? Owner->FindComponentByClass<UCombatStateComponent>() : nullptr;
	const float DamageAfterArmor = ComputeDamageAfterArmor(Hit.Damage, BaseArmor,
		States && States->HasState(GameTags::State_Combat_ArmorBroken), UGameTuningSettings::Get()->ArmorBrokenArmorMultiplier);
	const float Applied = FMath::Min(DamageAfterArmor, CurrentHealth);
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
