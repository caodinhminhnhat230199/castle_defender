#include "Hero/StaminaComponent.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	Config = FStaminaConfig();
	State.Init(Config);
}

void UStaminaComponent::InitializeComponent()
{
	Super::InitializeComponent();
	State.Init(Config);
	UpdateTickState();
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner()))
	{
		if (const UHeroClassDefinition* Def = Hero->GetHeroClassDefinition())
		{
			InitializeFromConfig(Def->Stamina);
		}
	}
}

void UStaminaComponent::InitializeFromConfig(const FStaminaConfig& InConfig)
{
	Config = InConfig;
	State.Init(Config);
	UpdateTickState();
	OnStaminaChanged.Broadcast(State.Current, State.Max);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsRegistered())
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

	if (bInfiniteStamina)
	{
		State.Current = State.Max;
		UpdateTickState();
		return;
	}

	HeroActionClock += (double)DeltaTime;
	const float PrevStamina = State.Current;

	if (bSprintDraining && Config.SprintDrainPerSecond > 0.f)
	{
		State.DrainSprint(DeltaTime, HeroActionClock, Config.SprintDrainPerSecond);
		if (State.Current <= 0.f)
		{
			OnStaminaDepleted.Broadcast();
			if (AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner()))
			{
				Hero->StopSprint();
			}
		}
	}

	State.Advance(DeltaTime, HeroActionClock, bBlocking, Config);

	if (!FMath::IsNearlyEqual(State.Current, PrevStamina, 0.01f))
	{
		OnStaminaChanged.Broadcast(State.Current, State.Max);
	}

	UpdateTickState();
}

bool UStaminaComponent::TrySpend(float Cost)
{
	if (bInfiniteStamina)
	{
		return true;
	}

	if (Cost <= 0.f)
	{
		return true;
	}

	if (!State.TrySpend(Cost, HeroActionClock))
	{
		OnStaminaSpendFailed.Broadcast(Cost);
		return false;
	}

	UpdateTickState();
	OnStaminaChanged.Broadcast(State.Current, State.Max);

	if (State.Current <= 0.f)
	{
		OnStaminaDepleted.Broadcast();
	}

	return true;
}

bool UStaminaComponent::ApplyDamage(float Amount)
{
	if (bInfiniteStamina)
	{
		return false;
	}

	const bool bDepleted = State.ApplyDamage(Amount, HeroActionClock);
	UpdateTickState();
	OnStaminaChanged.Broadcast(State.Current, State.Max);

	if (bDepleted)
	{
		OnStaminaDepleted.Broadcast();
	}

	return bDepleted;
}

void UStaminaComponent::OnBlockedHit(float Suppression)
{
	State.OnBlockedHit(HeroActionClock, Suppression);
	UpdateTickState();
}

void UStaminaComponent::SetBlocking(bool bInBlocking)
{
	bBlocking = bInBlocking;
	UpdateTickState();
}

void UStaminaComponent::SetSprintDraining(bool bInDraining)
{
	bSprintDraining = bInDraining;
	UpdateTickState();
}

void UStaminaComponent::SetInfiniteStamina(bool bEnabled)
{
	bInfiniteStamina = bEnabled;
	if (bInfiniteStamina)
	{
		State.Current = State.Max;
	}
	UpdateTickState();
	OnStaminaChanged.Broadcast(State.Current, State.Max);
}

void UStaminaComponent::UpdateTickState()
{
	const bool bNeedsTick = !bInfiniteStamina && (State.Current < State.Max || bSprintDraining);
	SetComponentTickEnabled(bNeedsTick);
}
