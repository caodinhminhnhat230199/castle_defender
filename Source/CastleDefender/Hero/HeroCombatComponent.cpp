#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroCharacter.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/MeleeTraceComponent.h"
#include "Hero/StaminaComponent.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Hero/HeroClassDefinition.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

UHeroCombatComponent::UHeroCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHeroCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	HeroOwner = Cast<AHeroCharacter>(GetOwner());
	if (HeroOwner)
	{
		HeroOwner->GetHealthComponent()->OnDamaged.AddDynamic(this, &UHeroCombatComponent::HandleOwnerDamaged);
		CombatStateComp = HeroOwner->GetCombatStateComponent();
		if (CombatStateComp)
		{
			CombatStateComp->OnStateAdded.AddDynamic(this, &UHeroCombatComponent::HandleStateAdded);
			CombatStateComp->OnStateRemoved.AddDynamic(this, &UHeroCombatComponent::HandleStateRemoved);
		}
		if (UMeleeTraceComponent* TraceComp = HeroOwner->FindComponentByClass<UMeleeTraceComponent>())
		{
			TraceComp->OnHitResolved.AddDynamic(this, &UHeroCombatComponent::HandleMeleeHitResolved);
		}
	}
}

void UHeroCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsRegistered()) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }
	// D-20: component DeltaTime already includes owner dilation; tick only while a buffered press needs an expiry clock.
	if (BufferedInput.bValid)
	{
		BufferedInput.Age += DeltaTime;
		const AHeroCharacter* Hero = GetHeroOwner();
		const float Limit = Hero && Hero->GetHeroClassDefinition() ? Hero->GetHeroClassDefinition()->Input.InputBufferTime : FHeroInputData().InputBufferTime;
		if (BufferedInput.Age > Limit) { ClearBuffer(); }
	}
}

#if !UE_BUILD_SHIPPING
FString UHeroCombatComponent::GetCombatDebugString() const
{
	const AHeroCharacter* Hero = GetHeroOwner();
	const UAnimInstance* Anim = Hero && Hero->GetMesh() ? Hero->GetMesh()->GetAnimInstance() : nullptr;
	const float Position = ActiveMontage && Anim ? Anim->Montage_GetPosition(ActiveMontage) : 0.f;
	FCombatActionTiming Timing;
	FString Error;
	FString Phase = TEXT("Inactive");
	if (ActiveMontage)
	{
		if (!FCombatActionTiming::InspectMontage(ActiveMontage, Timing, &Error)) { Phase = TEXT("Invalid: ") + Error; }
		else if (Timing.bHasHitWindow)
		{
			Phase = Position < Timing.HitWindowStart ? TEXT("Startup") : Position < Timing.HitWindowEnd ? TEXT("Active") : TEXT("Recovery");
		}
		else if (Timing.bHasInvulnerableWindow)
		{
			Phase = Position < Timing.InvulnerableWindowStart ? TEXT("Startup") : Position < Timing.InvulnerableWindowEnd ? TEXT("Active") : TEXT("Recovery");
		}
		else { Phase = bCancelWindowOpen ? TEXT("Recovery/Cancel") : TEXT("Committed"); }
	}
	FString Cancels;
	for (EHeroAction Action : OpenCancelActions) { Cancels += StaticEnum<EHeroAction>()->GetNameStringByValue(static_cast<int64>(Action)) + TEXT(" "); }
	return FString::Printf(TEXT("%s | Chain %d\n%s %.3f/%.3f (%s)\nHit %d IFrame %d Parry %d Resistance %d\nCancel %d [%s]\nBuffer %s age %.3f | Stamina %.1f/%.1f\nShared [%s]\nLock-on inactive | Assist inactive\nParry consumed inactive"),
		*StaticEnum<EHeroActionState>()->GetNameStringByValue(static_cast<int64>(CurrentState)), CurrentChainIndex, *GetNameSafe(ActiveMontage), Position, Timing.TotalDuration, *Phase,
		Hero && Hero->GetMeleeTraceComponent()->IsHitWindowActive(), bInvulnerableWindowOpen, bParryWindowOpen, bInterruptResistanceWindowOpen, bCancelWindowOpen, *Cancels,
		BufferedInput.bValid ? *StaticEnum<EHeroAction>()->GetNameStringByValue(static_cast<int64>(BufferedInput.Action)) : TEXT("None"), BufferedInput.Age,
		Hero ? Hero->GetStaminaComponent()->GetCurrentStamina() : 0.f, Hero ? Hero->GetStaminaComponent()->GetMaxStamina() : 0.f,
		CombatStateComp ? *CombatStateComp->GetActiveStates().ToStringSimple() : TEXT(""));
}
#endif

bool UHeroCombatComponent::IsSharedStaggered() const
{
	return CombatStateComp && CombatStateComp->HasState(GameTags::State_Combat_Staggered);
}

AHeroCharacter* UHeroCombatComponent::GetHeroOwner() const
{
	if (HeroOwner)
	{
		return HeroOwner;
	}
	return Cast<AHeroCharacter>(GetOwner());
}

float UHeroCombatComponent::GetActionStaminaCost(EHeroAction Action) const
{
	const AHeroCharacter* Hero = GetHeroOwner();
	if (!Hero)
	{
		return 0.f;
	}

	const UHeroClassDefinition* Def = Hero->GetHeroClassDefinition();
	if (!Def)
	{
		return 0.f;
	}

	switch (Action)
	{
	case EHeroAction::Light:
	{
		int32 TargetChainIndex = 0;
		if (CurrentState == EHeroActionState::LightAttack && bCancelWindowOpen && OpenCancelActions.Contains(EHeroAction::Light))
		{
			TargetChainIndex = FMath::Clamp(CurrentChainIndex + 1, 0, 2);
		}
		if (Def->LightChain.IsValidIndex(TargetChainIndex))
		{
			return Def->LightChain[TargetChainIndex].StaminaCost;
		}
		return 0.f;
	}
	case EHeroAction::Heavy:
		return Def->HeavyStaminaCost;
	case EHeroAction::Dodge:
		return Def->Dodge.StaminaCost;
	case EHeroAction::BlockStart:
	case EHeroAction::BlockEnd:
	case EHeroAction::Parry:
	case EHeroAction::Interact:
	default:
		return 0.f;
	}
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
	HeroOwner = GetHeroOwner();
	int32 TargetChainIndex = 0;
	if (Action == EHeroAction::Light)
	{
		if (CurrentState == EHeroActionState::LightAttack && bCancelWindowOpen && OpenCancelActions.Contains(EHeroAction::Light))
		{
			TargetChainIndex = FMath::Clamp(CurrentChainIndex + 1, 0, 2);
		}
	}

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
		if (Action == EHeroAction::Light)
		{
			const UHeroClassDefinition* Definition = HeroOwner ? HeroOwner->GetHeroClassDefinition() : nullptr;
			FString Error;
			if (!Definition || !Definition->ValidateLightAttack(TargetChainIndex, Error))
			{
				if (!Definition) { Error = TEXT("HeroClassDefinition is missing."); }
				if (LastLightValidationError != Error)
				{
					UE_LOG(LogGameCombat, Error, TEXT("Light attack refused: %s"), *Error);
					LastLightValidationError = Error;
				}
				return false;
			}
			LastLightValidationError.Reset();
		}

		if (Action == EHeroAction::Heavy)
		{
			// R-CMB-45: an invalid Heavy refuses to start instead of entering a state no montage will end.
			const UHeroClassDefinition* Definition = HeroOwner ? HeroOwner->GetHeroClassDefinition() : nullptr;
			FString Error;
			if (!Definition || !Definition->ValidateHeavyAttack(Error))
			{
				if (!Definition) { Error = TEXT("HeroClassDefinition is missing."); }
				if (LastHeavyValidationError != Error)
				{
					UE_LOG(LogGameCombat, Error, TEXT("Heavy attack refused: %s"), *Error);
					LastHeavyValidationError = Error;
				}
				return false;
			}
			LastHeavyValidationError.Reset();
		}

		if (Action == EHeroAction::Dodge)
		{
			const UHeroClassDefinition* Definition = HeroOwner ? HeroOwner->GetHeroClassDefinition() : nullptr;
			const FVector Input = HeroOwner ? HeroOwner->GetMovementInputWorldDirection() : FVector::ZeroVector;
			// Camera-facing hero picks the clip relative to its facing (T-CMB-10 adds lock-on); otherwise it turns to the input.
			LastDodgeDirection = FHeroDodgeData::SelectDirection(Input, HeroOwner ? HeroOwner->GetActorForwardVector() : FVector::ForwardVector,
				HeroOwner && HeroOwner->IsFacingCameraDirection());
			FString Error;
			if (!Definition || !Definition->ValidateDodge(LastDodgeDirection, Error))
			{
				if (!Definition) { Error = TEXT("HeroClassDefinition is missing."); }
				if (LastDodgeValidationError != Error)
				{
					UE_LOG(LogGameCombat, Error, TEXT("Dodge[%d] refused: %s"), static_cast<int32>(LastDodgeDirection), *Error);
					LastDodgeValidationError = Error;
				}
				return false;
			}
			LastDodgeValidationError.Reset();
		}

		ClearBuffer();
		ForceCloseAllWindows();
		if (Action != EHeroAction::Light && ActiveMontage && HeroOwner)
		{
			// A cancelled attack must not reopen hit windows from its remaining notifies.
			UAnimMontage* CancelledMontage = ActiveMontage;
			ActiveMontage = nullptr;
			HeroOwner->StopAnimMontage(CancelledMontage);
		}

		if (HeroOwner)
		{
			HeroOwner->StopSprint();
		}

		switch (Action)
		{
		case EHeroAction::Light:
		{
			CurrentChainIndex = TargetChainIndex;
			const FHeroAttackData* AttackData = GetAttackDataForAction(EHeroAction::Light, CurrentChainIndex);
			if (AttackData && HeroOwner)
			{
				ArmMeleeTrace(*AttackData, false);

				if (AttackData->Montage)
				{
					if (!PlayActionMontage(AttackData->Montage, EHeroActionState::LightAttack))
					{
						return false;
					}
				}
			}
			break;
		}
		case EHeroAction::Heavy:
		{
			CurrentChainIndex = 0;
			const FHeroAttackData& Heavy = HeroOwner->GetHeroClassDefinition()->Heavy;
			ArmMeleeTrace(Heavy, true);
			if (!PlayActionMontage(Heavy.Montage, EHeroActionState::HeavyAttack))
			{
				return false;
			}
			break;
		}
		case EHeroAction::Dodge:
		{
			CurrentChainIndex = 0;
			const FVector Input = HeroOwner->GetMovementInputWorldDirection();
			const bool bSideDodge = LastDodgeDirection == EHeroDodgeDirection::Left || LastDodgeDirection == EHeroDodgeDirection::Right;
			const bool bTurnToInput = !HeroOwner->IsFacingCameraDirection()
				|| (bSideDodge && HeroOwner->GetHeroClassDefinition()->Dodge.bSideClipsFaceInput);
			if (bTurnToInput && !Input.IsNearlyZero()) { HeroOwner->SetActorRotation(Input.Rotation()); }
			PreviousRootMotionScale = HeroOwner->GetAnimRootMotionTranslationScale();
			bDodgeRootMotionScaleApplied = true;
			HeroOwner->SetAnimRootMotionTranslationScale(HeroOwner->GetHeroClassDefinition()->Dodge.RootMotionScale);
			if (!PlayActionMontage(HeroOwner->GetHeroClassDefinition()->Dodge.GetMontage(LastDodgeDirection), EHeroActionState::Dodge))
			{
				HeroOwner->SetAnimRootMotionTranslationScale(PreviousRootMotionScale);
				bDodgeRootMotionScaleApplied = false;
				return false;
			}
			break;
		}
		case EHeroAction::BlockStart:
			CurrentChainIndex = 0;
			SetActionState(EHeroActionState::Block);
			break;
		case EHeroAction::BlockEnd:
			CurrentChainIndex = 0;
			SetActionState(EHeroActionState::Idle);
			break;
		case EHeroAction::Parry:
			CurrentChainIndex = 0;
			SetActionState(EHeroActionState::Parry);
			break;
		case EHeroAction::Interact:
			// Interact does not lock into an attack state
			break;
		}

		if (StaminaCost > 0.f && HeroOwner)
		{
			if (UStaminaComponent* Stamina = HeroOwner->GetStaminaComponent())
			{
				Stamina->TrySpend(StaminaCost);
			}
		}
		UE_LOG(LogGameCombat, Log, TEXT("Hero action %d accepted (Chain: %d) -> State: %d"),
			static_cast<int32>(Action), CurrentChainIndex, static_cast<int32>(CurrentState));
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
	if (OldState == EHeroActionState::Dodge && bDodgeRootMotionScaleApplied && HeroOwner)
	{
		HeroOwner->SetAnimRootMotionTranslationScale(PreviousRootMotionScale);
		bDodgeRootMotionScaleApplied = false;
	}
	CurrentState = NewState;

	if (CurrentState == EHeroActionState::Idle || CurrentState == EHeroActionState::Dead)
	{
		CurrentChainIndex = 0;
		ForceCloseAllWindows();
	}

	OnActionStateChanged.Broadcast(OldState, NewState);
}

void UHeroCombatComponent::ResetChain()
{
	CurrentChainIndex = 0;
}

const FHeroAttackData* UHeroCombatComponent::GetAttackDataForAction(EHeroAction Action, int32 ChainIndex) const
{
	if (!HeroOwner)
	{
		return nullptr;
	}

	const UHeroClassDefinition* ClassDef = HeroOwner->GetHeroClassDefinition();
	if (!ClassDef)
	{
		return nullptr;
	}

	if (Action == EHeroAction::Light)
	{
		if (ClassDef->LightChain.IsValidIndex(ChainIndex))
		{
			return &ClassDef->LightChain[ChainIndex];
		}
	}
	else if (Action == EHeroAction::Heavy)
	{
		return &ClassDef->Heavy;
	}

	return nullptr;
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
	if (CurrentState == EHeroActionState::LightAttack && OpenCancelActions.Contains(EHeroAction::Light))
	{
		ResetChain();
	}
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
	CloseInterruptResistanceWindow();

	if (HeroOwner)
	{
		if (UMeleeTraceComponent* TraceComp = HeroOwner->FindComponentByClass<UMeleeTraceComponent>())
		{
			TraceComp->EndHitWindow();
		}
	}
}

void UHeroCombatComponent::BufferAction(EHeroAction Action)
{
	BufferedInput.Action = Action;
	BufferedInput.Age = 0.f;
	SetComponentTickEnabled(true);
	BufferedInput.bValid = true;
}

void UHeroCombatComponent::ClearBuffer()
{
	BufferedInput = FBufferedAction();
	SetComponentTickEnabled(false);
}

void UHeroCombatComponent::TryConsumeBuffer()
{
	if (!BufferedInput.bValid)
	{
		return;
	}

	const float BufferWindow = (HeroOwner && HeroOwner->GetHeroClassDefinition()) ? HeroOwner->GetHeroClassDefinition()->Input.InputBufferTime : 0.2f;
	if (BufferedInput.Age <= BufferWindow)
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

void UHeroCombatComponent::ArmMeleeTrace(const FHeroAttackData& Attack, bool bHeavy) const
{
	UMeleeTraceComponent* TraceComp = HeroOwner ? HeroOwner->GetMeleeTraceComponent() : nullptr;
	if (!TraceComp)
	{
		return;
	}
	FCombatHit PendingHit;
	PendingHit.Damage = Attack.Damage;
	PendingHit.PoiseDamage = Attack.PoiseDamage;
	PendingHit.SourceLayer = ECombatLayer::Hero;
	PendingHit.DamageType = GameTags::Damage_Physical;
	PendingHit.bIsHeavy = bHeavy;
	PendingHit.AppliedStates = Attack.AppliedStates; // R-CMB-16: empty in P0, Armor Broken from P1
	PendingHit.StateDuration = Attack.StateDuration;
	PendingHit.Instigator = HeroOwner;
	TraceComp->SetPendingAttack(PendingHit, Attack.TraceRadius, FName(TEXT("Trace_Start")), FName(TEXT("Trace_End")));
}

bool UHeroCombatComponent::PlayActionMontage(UAnimMontage* Montage, EHeroActionState NewState)
{
	if (!Montage || !HeroOwner)
	{
		return false;
	}

	ForceCloseAllWindows();
	// Detach the previous owner before Montage_Play can interrupt its instance.
	ActiveMontage = nullptr;

	UAnimInstance* AnimInstance = HeroOwner->GetMesh() ? HeroOwner->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && HeroOwner->PlayAnimMontage(Montage) > 0.f)
	{
		ActiveMontage = Montage;
		SetActionState(NewState);
		FOnMontageEnded EndedDelegate;
		EndedDelegate.BindUObject(this, &UHeroCombatComponent::HandleMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndedDelegate, Montage);
		return true;
	}

	else
	{
		SetActionState(NewState == EHeroActionState::Dead ? EHeroActionState::Dead : EHeroActionState::Idle);
		UE_LOG(LogGameCombat, Warning, TEXT("Cannot play action montage %s: configure the hero mesh and animation instance."), *GetNameSafe(Montage));
		return false;
	}
}

void UHeroCombatComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ActiveMontage == Montage)
	{
		// A queued end from an older instance must not close a restarted montage of the same asset.
		if (HeroOwner && HeroOwner->GetMesh()->GetAnimInstance()
			&& HeroOwner->GetMesh()->GetAnimInstance()->Montage_IsActive(Montage)) { return; }
		ActiveMontage = nullptr;
		ForceCloseAllWindows();

		if (CurrentState != EHeroActionState::Dead)
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
			UAnimMontage* InterruptedMontage = ActiveMontage;
			ActiveMontage = nullptr;
			HeroOwner->StopAnimMontage(InterruptedMontage);
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

void UHeroCombatComponent::HandleOwnerDamaged(const FCombatHit& Hit, float NewHealth)
{
	if (!HeroOwner || NewHealth <= 0.f || HeroOwner->GetHealthComponent()->IsDead()
		|| CurrentState == EHeroActionState::Dead || Hit.bWasBlocked || Hit.Damage <= 0.f) { return; }
	const FHeroAttackData* Attack = nullptr;
	if (CurrentState == EHeroActionState::HeavyAttack) { Attack = GetAttackDataForAction(EHeroAction::Heavy); }
	else if (CurrentState == EHeroActionState::LightAttack) { Attack = GetAttackDataForAction(EHeroAction::Light, CurrentChainIndex); }
	const bool bResisted = Attack && bInterruptResistanceWindowOpen && Attack->bInterruptResistanceEnabled
		&& Hit.InterruptData.InterruptStrength < Attack->InterruptResistance;
	if (bResisted || !Hit.InterruptData.bCanInterrupt) { return; }

	ForceCloseAllWindows();
	ClearBuffer();
	if (ActiveMontage)
	{
		UAnimMontage* InterruptedMontage = ActiveMontage;
		ActiveMontage = nullptr;
		HeroOwner->StopAnimMontage(InterruptedMontage);
	}
	const FVector ToSource = Hit.Instigator.IsValid()
		? Hit.Instigator->GetActorLocation() - HeroOwner->GetActorLocation() : -Hit.HitDirection;
	const bool bFromFront = FVector::DotProduct(HeroOwner->GetActorForwardVector(), ToSource.GetSafeNormal2D()) >= 0.f;
	const UHeroClassDefinition* Definition = HeroOwner->GetHeroClassDefinition();
	UAnimMontage* Reaction = Definition ? (bFromFront ? Definition->HitReact.FrontMontage : Definition->HitReact.BackMontage) : nullptr;
	FString Error;
	const bool bValidReaction = Definition && Definition->ValidateHitReaction(bFromFront, Error);
	if (!Definition) { Error = TEXT("HeroClassDefinition is missing."); }
	if (!bValidReaction && LastReactionValidationError != Error)
	{
		UE_LOG(LogGameCombat, Error, TEXT("Hit reaction refused: %s"), *Error);
		LastReactionValidationError = Error;
	}
	if (bValidReaction) { LastReactionValidationError.Reset(); }
	if (!bValidReaction || !Reaction || !PlayActionMontage(Reaction, EHeroActionState::HitReact))
	{
		SetActionState(EHeroActionState::Idle);
	}
	HeroOwner->OnHitReactPresentation(Hit, bFromFront);
	// DeliverHit is the sole producer of hit feedback; presentation does not replay HeroDamaged.
}

void UHeroCombatComponent::HandleOwnerDeath(const FCombatHit& KillingHit)
{
	ForceCloseAllWindows();
	ClearBuffer();
	if (ActiveMontage && HeroOwner)
	{
		UAnimMontage* InterruptedMontage = ActiveMontage;
		ActiveMontage = nullptr;
		HeroOwner->StopAnimMontage(InterruptedMontage);
	}
	SetActionState(EHeroActionState::Dead);
	if (HeroOwner && HeroOwner->GetHeroClassDefinition() && HeroOwner->GetHeroClassDefinition()->HitReact.DeathMontage)
	{
		PlayActionMontage(HeroOwner->GetHeroClassDefinition()->HitReact.DeathMontage, EHeroActionState::Dead);
	}
}

ECombatHitResult UHeroCombatComponent::InterceptHit(FCombatHit& Hit)
{
	if (IsInInvulnerableWindow())
	{
		return ECombatHitResult::Evaded;
	}

	if (IsInParryWindow())
	{
		// TODO: T-CMB-09 full parry counter and poise damage
		return ECombatHitResult::Parried;
	}

	return ECombatHitResult::Hit;
}

void UHeroCombatComponent::NotifyCombatResolved(const FCombatResolutionEvent& Event)
{
	OnCombatResolved.Broadcast(Event);
}

void UHeroCombatComponent::HandleMeleeHitResolved(AActor* Target, ECombatHitResult Result)
{
	OnHitLanded.Broadcast(Target, Result);
}
