#include "Combat/CombatStateModel.h"

#include "Core/GameTags.h"

void FCombatStateModel::Init(const FCombatStateConfig& InConfig)
{
	Config = InConfig;
	States.Reset();
	RefillPoise();
}

void FCombatStateModel::RefillPoise()
{
	StoredPoise = Config.MaxPoise;
	LastPoiseDamageTime = -UE_BIG_NUMBER;
}

float FCombatStateModel::GetPoise(double Now) const
{
	if (Config.MaxPoise <= 0.f || HasState(GameTags::State_Combat_Staggered))
	{
		return 0.f;
	}
	const double RegenTime = FMath::Max(0.0, Now - LastPoiseDamageTime - Config.PoiseRegenDelay);
	return static_cast<float>(FMath::Min<double>(Config.MaxPoise, StoredPoise + RegenTime * Config.PoiseRegenRate));
}

void FCombatStateModel::SetPoise(float Value, double Now)
{
	if (Config.MaxPoise <= 0.f || HasState(GameTags::State_Combat_Staggered))
	{
		return;
	}
	StoredPoise = FMath::Clamp(Value, 0.f, Config.MaxPoise);
	LastPoiseDamageTime = Now;
}

bool FCombatStateModel::ApplyPoiseDamage(float Amount, double Now)
{
	if (Config.MaxPoise <= 0.f || Amount <= 0.f || HasState(GameTags::State_Combat_Staggered))
	{
		return false;
	}
	StoredPoise = FMath::Max(0.f, GetPoise(Now) - Amount);
	LastPoiseDamageTime = Now;
	return StoredPoise <= 0.f;
}

FCombatStateModel::EApplyResult FCombatStateModel::ApplyState(FGameplayTag State, float Duration, AActor* Instigator, double Now)
{
	const double Expiry = Now + Duration;
	if (FActiveCombatState* Active = States.FindByPredicate([State](const FActiveCombatState& S) { return S.StateTag == State; }))
	{
		Active->ExpiryTime = FMath::Max(Active->ExpiryTime, Expiry);
		Active->Instigator = Instigator;
		return EApplyResult::Refreshed;
	}
	FActiveCombatState& Added = States.AddDefaulted_GetRef();
	Added.StateTag = State;
	Added.ExpiryTime = Expiry;
	Added.Instigator = Instigator;
	return EApplyResult::Added;
}

bool FCombatStateModel::RemoveState(FGameplayTag State)
{
	if (States.RemoveAll([State](const FActiveCombatState& S) { return S.StateTag == State; }) == 0)
	{
		return false;
	}
	if (State == GameTags::State_Combat_Staggered)
	{
		RefillPoise();
	}
	return true;
}

TArray<FGameplayTag> FCombatStateModel::RemoveExpired(double Now)
{
	TArray<FGameplayTag> Expired;
	for (const FActiveCombatState& State : States)
	{
		if (State.ExpiryTime <= Now)
		{
			Expired.Add(State.StateTag);
		}
	}
	for (const FGameplayTag& Tag : Expired)
	{
		RemoveState(Tag);
	}
	return Expired;
}

TArray<FGameplayTag> FCombatStateModel::ClearAll()
{
	TArray<FGameplayTag> Removed;
	for (const FActiveCombatState& State : States)
	{
		Removed.Add(State.StateTag);
	}
	States.Reset();
	RefillPoise();
	return Removed;
}

TOptional<double> FCombatStateModel::NextExpiry() const
{
	TOptional<double> Earliest;
	for (const FActiveCombatState& State : States)
	{
		if (!Earliest || State.ExpiryTime < *Earliest)
		{
			Earliest = State.ExpiryTime;
		}
	}
	return Earliest;
}

bool FCombatStateModel::HasState(FGameplayTag State) const
{
	return States.ContainsByPredicate([State](const FActiveCombatState& S) { return S.StateTag.MatchesTag(State); });
}

const FActiveCombatState* FCombatStateModel::FindState(FGameplayTag State) const
{
	return States.FindByPredicate([State](const FActiveCombatState& S) { return S.StateTag == State; });
}
