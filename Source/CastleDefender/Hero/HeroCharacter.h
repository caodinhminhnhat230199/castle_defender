#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "Combat/CombatTypes.h"
#include "Hero/HeroClassDefinition.h"
#include "HeroCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UHealthComponent;
class UCombatStateComponent;
class UHeroCombatComponent;
class UStaminaComponent;
class UMeleeTraceComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeroDeathSignature, const FCombatHit&, KillingHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHeroFeedbackRequestedSignature, FGameplayTag, FeedbackTag, const FCombatHit&, Hit);

/**
 * Playable Warlord hero character (spec §4.4, D-19, D-20).
 * Locomotion, sprint, third-person orbit camera, and foundation combat contracts.
 */
UCLASS()
class CASTLEDEFENDER_API AHeroCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AHeroCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IGenericTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { TeamId = NewTeamId; }

	/** Applies tunables from HeroClassDefinition (or defaults). Safe to call at runtime via cheat. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void ApplyTuning();

	/** Initiates sprint locomotion. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void StartSprint();

	/** Stops sprint locomotion and returns to jog speed. */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void StopSprint();

	UFUNCTION(BlueprintPure, Category = "Hero")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	FVector GetMovementInputWorldDirection() const;

	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroClassDefinition* GetHeroClassDefinition() const { return HeroClassDefinition; }

	UFUNCTION(BlueprintCallable, Category = "Hero")
	void SetHeroClassDefinition(UHeroClassDefinition* InDef) { HeroClassDefinition = InDef; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UHealthComponent* GetHealthComponent() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UCombatStateComponent* GetCombatStateComponent() const { return CombatState; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UHeroCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UMeleeTraceComponent* GetMeleeTraceComponent() const { return MeleeTraceComponent; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintPure, Category = "Hero")
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(BlueprintAssignable, Category = "Hero")
	FHeroDeathSignature OnHeroDeath;
	UPROPERTY(BlueprintAssignable, Category = "Hero")
	FHeroFeedbackRequestedSignature OnFeedbackRequested;
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero|Presentation")
	void OnHitReactPresentation(const FCombatHit& Hit, bool bFromFront);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero|Presentation")
	void OnDeathPresentation(const FCombatHit& Hit);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatStateComponent> CombatState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHeroCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMeleeTraceComponent> MeleeTraceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	TObjectPtr<UHeroClassDefinition> HeroClassDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero")
	FGenericTeamId TeamId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BlockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ParryAction;

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);
	void OnLightAttack(const FInputActionValue& Value);
	void OnHeavyAttack(const FInputActionValue& Value);
	void OnDodge(const FInputActionValue& Value);
	void OnBlockStarted(const FInputActionValue& Value);
	void OnBlockCompleted(const FInputActionValue& Value);
	void OnParry(const FInputActionValue& Value);

private:
	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit);

	void UpdateMaxWalkSpeed();

	bool bIsSprinting = false;
	bool bDeathHandled = false;
	FVector2D MovementInputAxes = FVector2D::ZeroVector;
};
