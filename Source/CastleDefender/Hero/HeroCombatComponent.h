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

	const TArray<EHeroAction>& GetAllowedCancelActions() const { return OpenCancelActions; }

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

	/** Plays a montage owning a given action state, wiring blend-out and completion to return to Idle. */
	void PlayActionMontage(UAnimMontage* Montage, EHeroActionState NewState);

	/** Force-closes all active windows. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ForceCloseAllWindows();

	/** Clears any buffered input. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearBuffer();

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
		double Timestamp = 0.0;
		bool bValid = false;
	};

	void SetActionState(EHeroActionState NewState);
	void TryConsumeBuffer();
	void BufferAction(EHeroAction Action);
	bool IsSharedStaggered() const;

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleStateAdded(FGameplayTag StateTag, AActor* Instigator);

	UFUNCTION()
	void HandleStateRemoved(FGameplayTag StateTag);

	UFUNCTION()
	void HandleOwnerDeath(const FCombatHit& KillingHit);

	FBufferedAction BufferedInput;

	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	UPROPERTY()
	TObjectPtr<AHeroCharacter> HeroOwner;

	UPROPERTY()
	TObjectPtr<UCombatStateComponent> CombatStateComp;
};
