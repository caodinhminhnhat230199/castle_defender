#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroCharacter.h"
#include "Combat/CombatStateComponent.h"
#include "Hero/StaminaComponent.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

UHeroCombatComponent::UHeroCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHeroCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	HeroOwner = Cast<AHeroCharacter>(GetOwner());
	if (HeroOwner)
	{
		HeroOwner->OnHeroDeath.AddDynamic(this, &UHeroCombatComponent::HandleOwnerDeath);
		CombatStateComp = HeroOwner->GetCombatStateComponent();
		if (CombatStateComp)
		{
			CombatStateComp->OnStateAdded.AddDynamic(this, &UHeroCombatComponent::HandleStateAdded);
			CombatStateComp->OnStateRemoved.AddDynamic(this, &UHeroCombatComponent::HandleStateRemoved);
		}
	}
}

bool UHeroCombatComponent::IsSharedStaggered() const
{
	return CombatStateComp && CombatStateComp->HasState(GameTags::State_Combat_Staggered);
}

float UHeroCombatComponent::GetActionStaminaCost(EHeroAction Action) const
{
	if (HeroOwner)
	{
		if (const UHeroClassDefinition* Def = HeroOwner->GetHeroClassDefinition())
		{
			switch (Action)
			{
			case EHeroAction::Dodge:
				return Def->DodgeStaminaCost;
			case EHeroAction::Heavy:
				return Def->HeavyStaminaCost;
			default:
				return 0.f;
			}
		}
	}
	return 0.f;
}

bool UHeroCombatComponent::CanStartAction(EHeroAction Action) const
{
	bool bStaminaOk = true;
	const float Cost = GetActionStaminaCost(Action);
	if (Cost > 0.f && HeroOwner)
	{
		if (const UStaminaComponent* Stamina = HeroOwner->GetStaminaComponent())
		{
			bStaminaOk = Stamina->HasInfiniteStamina() || (Stamina->GetCurrentStamina() >= Cost);
		}
	}

	const bool bStaggered = IsSharedStaggered();
	return FHeroActionRules::CanStart(CurrentState, OpenCancelActions, Action, bStaminaOk, bStaggered);
}

bool UHeroCombatComponent::RequestAction(EHeroAction Action)
{
	const float StaminaCost = GetActionStaminaCost(Action);
	if (StaminaCost > 0.f && HeroOwner)
	{
		if (UStaminaComponent* Stamina = HeroOwner->GetStaminaComponent())
		{
			if (!Stamina->HasInfiniteStamina() && Stamina->GetCurrentStamina() < StaminaCost)
			{
				Stamina->TrySpend(StaminaCost); // emits OnStaminaSpendFailed
				UE_LOG(LogGameCombat, Log, TEXT("Hero action %d rejected: stamina insufficient (cost %.1f, current %.1f) - not buffered"),
					static_cast<int32>(Action), StaminaCost, Stamina->GetCurrentStamina());
				return false;
			}
		}
	}

	if (CanStartAction(Action))
	{
		ClearBuffer();

		if (HeroOwner)
		{
			HeroOwner->StopSprint();
			if (StaminaCost > 0.f)
			{
				if (UStaminaComponent* Stamina = HeroOwner->GetStaminaComponent())
				{
					Stamina->TrySpend(StaminaCost);
				}
			}
		}

		switch (Action)
		{
		case EHeroAction::Light:
			SetActionState(EHeroActionState::LightAttack);
			break;
		case EHeroAction::Heavy:
			SetActionState(EHeroActionState::HeavyAttack);
			break;
		case EHeroAction::Dodge:
			SetActionState(EHeroActionState::Dodge);
			break;
		case EHeroAction::BlockStart:
			SetActionState(EHeroActionState::Block);
			break;
		case EHeroAction::BlockEnd:
			SetActionState(EHeroActionState::Idle);
			break;
		case EHeroAction::Parry:
			SetActionState(EHeroActionState::Parry);
			break;
		case EHeroAction::Interact:
			// Interact does not lock into an attack state
			break;
		}

		UE_LOG(LogGameCombat, Log, TEXT("Hero action %d accepted -> State: %d"), static_cast<int32>(Action), static_cast<int32>(CurrentState));
		return true;
	}

	// Refuse and optionally buffer
	if (CurrentState != EHeroActionState::Dead && !IsSharedStaggered())
	{
		BufferAction(Action);
	}

	UE_LOG(LogGameCombat, Verbose, TEXT("Hero action %d rejected in state %d (buffered: %s)"),
		static_cast<int32>(Action), static_cast<int32>(CurrentState), BufferedInput.bValid ? TEXT("yes") : TEXT("no"));

	return false;
}

void UHeroCombatComponent::SetActionState(EHeroActionState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EHeroActionState OldState = CurrentState;
	CurrentState = NewState;

	if (CurrentState == EHeroActionState::Idle || CurrentState == EHeroActionState::Dead)
	{
		ForceCloseAllWindows();
	}

	OnActionStateChanged.Broadcast(OldState, NewState);
}

FGameplayTag UHeroCombatComponent::GetActionTag() const
{
	switch (CurrentState)
	{
	case EHeroActionState::LightAttack:
	case EHeroActionState::HeavyAttack:
		return GameTags::State_Hero_Attacking;
	case EHeroActionState::Dodge:
		return GameTags::State_Hero_Dodging;
	case EHeroActionState::Block:
		return GameTags::State_Hero_Blocking;
	case EHeroActionState::Parry:
		return GameTags::State_Hero_Parrying;
	case EHeroActionState::Dead:
		return GameTags::State_Hero_Dead;
	case EHeroActionState::Idle:
	case EHeroActionState::HitReact:
	default:
		return FGameplayTag();
	}
}

void UHeroCombatComponent::OpenCancelWindow(const TArray<EHeroAction>& InAllowedActions)
{
	bCancelWindowOpen = true;
	OpenCancelActions = InAllowedActions;
	TryConsumeBuffer();
}

void UHeroCombatComponent::CloseCancelWindow()
{
	bCancelWindowOpen = false;
	OpenCancelActions.Empty();
}

void UHeroCombatComponent::OpenInvulnerableWindow()
{
	bInvulnerableWindowOpen = true;
}

void UHeroCombatComponent::CloseInvulnerableWindow()
{
	bInvulnerableWindowOpen = false;
}

void UHeroCombatComponent::OpenParryWindow()
{
	bParryWindowOpen = true;
}

void UHeroCombatComponent::CloseParryWindow()
{
	bParryWindowOpen = false;
}

void UHeroCombatComponent::ForceCloseAllWindows()
{
	CloseCancelWindow();
	CloseInvulnerableWindow();
	CloseParryWindow();
}

void UHeroCombatComponent::BufferAction(EHeroAction Action)
{
	BufferedInput.Action = Action;
	BufferedInput.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	BufferedInput.bValid = true;
}

void UHeroCombatComponent::ClearBuffer()
{
	BufferedInput = FBufferedAction();
}

void UHeroCombatComponent::TryConsumeBuffer()
{
	if (!BufferedInput.bValid)
	{
		return;
	}

	const float BufferWindow = (HeroOwner && HeroOwner->GetHeroClassDefinition()) ? HeroOwner->GetHeroClassDefinition()->Input.InputBufferTime : 0.2f;
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	if ((Now - BufferedInput.Timestamp) <= BufferWindow)
	{
		const EHeroAction BufferedAct = BufferedInput.Action;
		if (CanStartAction(BufferedAct))
		{
			ClearBuffer();
			RequestAction(BufferedAct);
			return;
		}
	}
	else
	{
		// Buffer expired
		ClearBuffer();
	}
}

void UHeroCombatComponent::PlayActionMontage(UAnimMontage* Montage, EHeroActionState NewState)
{
	if (!Montage || !HeroOwner)
	{
		return;
	}

	ForceCloseAllWindows();
	SetActionState(NewState);
	ActiveMontage = Montage;

	UAnimInstance* AnimInstance = HeroOwner->GetMesh() ? HeroOwner->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		FOnMontageEnded EndedDelegate;
		EndedDelegate.BindUObject(this, &UHeroCombatComponent::HandleMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndedDelegate, Montage);
	}

	HeroOwner->PlayAnimMontage(Montage);
}

void UHeroCombatComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ActiveMontage == Montage)
	{
		ActiveMontage = nullptr;
		ForceCloseAllWindows();

		if (CurrentState != EHeroActionState::Dead && CurrentState != EHeroActionState::HitReact)
		{
			SetActionState(EHeroActionState::Idle);
			TryConsumeBuffer();
		}
	}
}

void UHeroCombatComponent::HandleStateAdded(FGameplayTag StateTag, AActor* Instigator)
{
	if (StateTag.MatchesTag(GameTags::State_Combat_Staggered))
	{
		ForceCloseAllWindows();
		ClearBuffer();

		if (ActiveMontage && HeroOwner)
		{
			HeroOwner->StopAnimMontage(ActiveMontage);
			ActiveMontage = nullptr;
		}

		if (CurrentState != EHeroActionState::Dead)
		{
			SetActionState(EHeroActionState::Idle);
		}
	}
}

void UHeroCombatComponent::HandleStateRemoved(FGameplayTag StateTag)
{
	if (StateTag.MatchesTag(GameTags::State_Combat_Staggered))
	{
		if (CurrentState == EHeroActionState::Idle)
		{
			TryConsumeBuffer();
		}
	}
}

void UHeroCombatComponent::HandleOwnerDeath(const FCombatHit& KillingHit)
{
	ForceCloseAllWindows();
	ClearBuffer();

	if (ActiveMontage && HeroOwner)
	{
		HeroOwner->StopAnimMontage(ActiveMontage);
		ActiveMontage = nullptr;
	}

	SetActionState(EHeroActionState::Dead);
}
