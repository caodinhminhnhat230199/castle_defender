#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"
#include "HeroCombatTypes.generated.h"

/**
 * Data definition for an individual hero attack (spec §4.4, technical-plan §3.3).
 * Configures montage, damage, poise damage, stamina cost, trace radius, and applied states.
 */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FHeroAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float PoiseDamage = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float StaminaCost = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float TraceRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	FGameplayTagContainer AppliedStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float StateDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Interruption")
	bool bInterruptResistanceEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Interruption", meta = (ClampMin = "0"))
	float InterruptResistance = 0.f;
};

/** Hero combat actions requested by input or AI (technical-plan §5.1). */
UENUM(BlueprintType)
enum class EHeroAction : uint8
{
	Light,
	Heavy,
	Dodge,
	BlockStart,
	BlockEnd,
	Parry,
	Interact
};

/**
 * State machine states owned by UHeroCombatComponent (technical-plan §5.1).
 * Staggered is a derived label from UCombatStateComponent::HasState, never an owned state.
 */
UENUM(BlueprintType)
enum class EHeroActionState : uint8
{
	Idle,
	LightAttack,
	HeavyAttack,
	Dodge,
	Block,
	Parry,
	HitReact,
	Dead
};

UENUM(BlueprintType)
enum class EHeroDodgeDirection : uint8 { Forward, Backward, Left, Right };

USTRUCT(BlueprintType)
struct FHeroHitReactData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UAnimMontage> FrontMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UAnimMontage> BackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UAnimMontage> DeathMontage;
};

/** Directional montages own movement/timing; the definition owns cost and distance scale. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FHeroDodgeData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> ForwardMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> BackwardMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> LeftMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> RightMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0"))
	float StaminaCost = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge", meta = (ClampMin = "0.001"))
	float RootMotionScale = 1.f;

	UAnimMontage* GetMontage(EHeroDodgeDirection Direction) const
	{
		switch (Direction)
		{
		case EHeroDodgeDirection::Backward: return BackwardMontage;
		case EHeroDodgeDirection::Left: return LeftMontage;
		case EHeroDodgeDirection::Right: return RightMontage;
		default: return ForwardMontage;
		}
	}
	static EHeroDodgeDirection SelectDirection(const FVector& WorldInput, const FVector& Facing, bool bLockedOn)
	{
		if (WorldInput.IsNearlyZero()) { return EHeroDodgeDirection::Backward; }
		if (!bLockedOn) { return EHeroDodgeDirection::Forward; }
		const FVector Forward = Facing.GetSafeNormal2D();
		const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
		const float F = FVector::DotProduct(WorldInput, Forward);
		const float R = FVector::DotProduct(WorldInput, Right);
		return FMath::Abs(F) >= FMath::Abs(R)
			? (F >= 0.f ? EHeroDodgeDirection::Forward : EHeroDodgeDirection::Backward)
			: (R >= 0.f ? EHeroDodgeDirection::Right : EHeroDodgeDirection::Left);
	}
};

/** Locomotion and sprint tunables for hero classes (spec §4.4). */
USTRUCT(BlueprintType)
struct FHeroMovementData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float JogSpeed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float RotationRateYaw = 720.f;
};

/** Orbit camera tunables for hero classes (spec §4.4). */
USTRUCT(BlueprintType)
struct FHeroCameraData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float TargetArmLength = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bEnableCameraLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraLagSpeed = 10.f;
};

/** Input buffering and timing tunables for hero classes (spec §4.4). */
USTRUCT(BlueprintType)
struct FHeroInputData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (ClampMin = "0.0"))
	float InputBufferTime = 0.2f;
};

/** Stamina tunables for hero classes (spec §4.4, technical-plan §5.2). */
USTRUCT(BlueprintType)
struct FStaminaConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float Max = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float RegenDelay = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float RegenRate = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float BlockingRegenMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float SprintDrainPerSecond = 0.f;
};

/** Pure stamina state and simulation rules (technical-plan §5.2). */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FStaminaState
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	float Current = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	float Max = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	double LastSpendTime = -100.0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stamina")
	double BlockedRegenUntil = 0.0;

	void Init(const FStaminaConfig& Config)
	{
		Max = Config.Max;
		Current = Max;
		LastSpendTime = -100.0;
		BlockedRegenUntil = 0.0;
	}

	bool TrySpend(float Cost, double Now)
	{
		if (Cost > Current)
		{
			return false;
		}
		Current -= Cost;
		LastSpendTime = Now;
		return true;
	}

	bool ApplyDamage(float Amount, double Now)
	{
		Current = FMath::Max(0.f, Current - Amount);
		LastSpendTime = Now;
		return Current == 0.f;
	}

	void OnBlockedHit(double Now, float Suppression)
	{
		BlockedRegenUntil = Now + (double)Suppression;
	}

	void Advance(float Dt, double Now, bool bBlocking, const FStaminaConfig& Config)
	{
		if (Dt <= 0.f)
		{
			return;
		}

		const double RegenStartTime = LastSpendTime + (double)Config.RegenDelay;
		const double AllowedStartTime = bBlocking ? FMath::Max(RegenStartTime, BlockedRegenUntil) : RegenStartTime;

		if (Now >= AllowedStartTime)
		{
			const float ActiveTime = FMath::Min(Dt, (float)(Now - AllowedStartTime));
			if (ActiveTime > 0.f)
			{
				const float Multiplier = bBlocking ? Config.BlockingRegenMultiplier : 1.0f;
				Current = FMath::Min(Config.Max, Current + Config.RegenRate * Multiplier * ActiveTime);
			}
		}
	}

	void DrainSprint(float Dt, double Now, float DrainRate)
	{
		if (DrainRate > 0.f && Dt > 0.f)
		{
			Current = FMath::Max(0.f, Current - DrainRate * Dt);
			LastSpendTime = Now;
		}
	}
};

/** Pure rules for hero action transitions and commitment windows (technical-plan §5.1). */
struct CASTLEDEFENDER_API FHeroActionRules
{
	/**
	 * Returns true if the requested action can start under the current state, open cancel windows,
	 * stamina availability and shared combat states.
	 */
	static bool CanStart(
		EHeroActionState CurrentState,
		const TArray<EHeroAction>& AllowedByOpenWindow,
		EHeroAction RequestedAction,
		bool bStaminaOk = true,
		bool bSharedStaggered = false)
	{
		// Dead hero dominates all rules - cannot act.
		if (CurrentState == EHeroActionState::Dead)
		{
			return false;
		}

		// Shared Staggered state suppresses all actions.
		if (bSharedStaggered)
		{
			return false;
		}

		// Stamina gate: actions cannot start if stamina requirement failed.
		// Note: BlockEnd does not require stamina.
		if (!bStaminaOk && RequestedAction != EHeroAction::BlockEnd)
		{
			return false;
		}

		switch (CurrentState)
		{
		case EHeroActionState::Idle:
			// In Idle, all actions can start except BlockEnd (which requires being in Block).
			return RequestedAction != EHeroAction::BlockEnd;

		case EHeroActionState::Block:
			// Releasing block is always allowed.
			if (RequestedAction == EHeroAction::BlockEnd)
			{
				return true;
			}
			// Other actions in Block only if an open window specifically allows it.
			return AllowedByOpenWindow.Contains(RequestedAction);

		case EHeroActionState::HitReact:
			// Hit reaction cannot be canceled unless an open cancel window explicitly allows it.
			return AllowedByOpenWindow.Contains(RequestedAction);

		case EHeroActionState::LightAttack:
		case EHeroActionState::HeavyAttack:
		case EHeroActionState::Dodge:
		case EHeroActionState::Parry:
			// Action commitment: only allowed if the active montage opened a cancel window containing the action.
			return AllowedByOpenWindow.Contains(RequestedAction);

		case EHeroActionState::Dead:
		default:
			return false;
		}
	}
};
