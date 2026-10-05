#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Hero/HeroCombatTypes.h"
#include "Combat/CombatHitInterceptor.h"
#include "Combat/CombatTypes.h"
#include "HeroCombatComponent.generated.h"

class UAnimMontage;
class AHeroCharacter;
class UCombatStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHeroActionStateChangedSignature, EHeroActionState, OldState, EHeroActionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatResolvedSignature, const FCombatResolutionEvent&, ResolutionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitLandedSignature, AActor*, Target, ECombatHitResult, Result);

/**
 * Action state machine and commitment manager for the Hero (spec §4.4, technical-plan §5.1).
 * Manages action commitment, cancel/invulnerable/parry windows, and input buffering.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UHeroCombatComponent : public UActorComponent, public ICombatHitInterceptor
{
	GENERATED_BODY()

public:
	UHeroCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempts to start a requested action. Buffers if currently committed but cancel window may open soon. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool RequestAction(EHeroAction Action);

	/** Returns true if the action can start immediately under current state and windows. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanStartAction(EHeroAction Action) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetActionStaminaCost(EHeroAction Action) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	EHeroActionState GetActionState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	FGameplayTag GetActionTag() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCancelWindow() const { return bCancelWindowOpen; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInInvulnerableWindow() const { return bInvulnerableWindowOpen; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInParryWindow() const { return bParryWindowOpen; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentChainIndex() const { return CurrentChainIndex; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetChain();

	const FHeroAttackData* GetAttackDataForAction(EHeroAction Action, int32 ChainIndex = 0) const;

	const TArray<EHeroAction>& GetAllowedCancelActions() const { return OpenCancelActions; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	AHeroCharacter* GetHeroOwner() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	EHeroDodgeDirection GetLastDodgeDirection() const { return LastDodgeDirection; }
	bool HasBufferedInput() const { return BufferedInput.bValid; }
	float GetBufferedInputAge() const { return BufferedInput.Age; }
#if !UE_BUILD_SHIPPING
	/** Read-only view of owner state and actual montage position; never maintains display timers. */
	FString GetCombatDebugString() const;
#endif

	// Window management called by AnimNotifyStates
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenCancelWindow(const TArray<EHeroAction>& InAllowedActions);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseCancelWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenInvulnerableWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseInvulnerableWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenParryWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseParryWindow();
	void OpenInterruptResistanceWindow() { bInterruptResistanceWindowOpen = true; }
	void CloseInterruptResistanceWindow() { bInterruptResistanceWindowOpen = false; }

	/** Plays a montage owning a given action state, wiring blend-out and completion to return to Idle. */
	bool PlayActionMontage(UAnimMontage* Montage, EHeroActionState NewState);

	/** Force-closes all active windows. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ForceCloseAllWindows();

	/** Clears any buffered input. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearBuffer();
	/** Called by the owning hero before publishing its death event to observers. */
	void HandleOwnerDeath(const FCombatHit& KillingHit);

	// ICombatHitInterceptor
	virtual ECombatHitResult InterceptHit(FCombatHit& Hit) override;
	virtual void NotifyCombatResolved(const FCombatResolutionEvent& Event) override;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FHeroActionStateChangedSignature OnActionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatResolvedSignature OnCombatResolved;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitLandedSignature OnHitLanded;

	UFUNCTION()
	void HandleMeleeHitResolved(AActor* Target, ECombatHitResult Result);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	EHeroActionState CurrentState = EHeroActionState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	int32 CurrentChainIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bCancelWindowOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bInvulnerableWindowOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bParryWindowOpen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	TArray<EHeroAction> OpenCancelActions;

private:
	struct FBufferedAction
	{
		EHeroAction Action = EHeroAction::Light;
		float Age = 0.f;
		bool bValid = false;
	};

	void SetActionState(EHeroActionState NewState);
	void TryConsumeBuffer();
	void BufferAction(EHeroAction Action);
	bool IsSharedStaggered() const;
	/** Loads the attack's hit payload into the melee trace for the montage's hit window. */
	void ArmMeleeTrace(const FHeroAttackData& Attack, bool bHeavy) const;

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleStateAdded(FGameplayTag StateTag, AActor* Instigator);

	UFUNCTION()
	void HandleStateRemoved(FGameplayTag StateTag);

	UFUNCTION()
	void HandleOwnerDamaged(const FCombatHit& Hit, float NewHealth);

	FBufferedAction BufferedInput;
	FString LastLightValidationError;
	FString LastHeavyValidationError;
	FString LastDodgeValidationError;
	FString LastReactionValidationError;
	EHeroDodgeDirection LastDodgeDirection = EHeroDodgeDirection::Backward;
	float PreviousRootMotionScale = 1.f;
	bool bDodgeRootMotionScaleApplied = false;
	bool bInterruptResistanceWindowOpen = false;

	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	UPROPERTY()
	TObjectPtr<AHeroCharacter> HeroOwner;

	UPROPERTY()
	TObjectPtr<UCombatStateComponent> CombatStateComp;
};
