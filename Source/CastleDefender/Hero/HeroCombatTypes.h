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

/** R-CMB-46/48: facing assistance is bounded by the attack's original intent; never adds translation. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FHeroAttackAssistData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Assist", meta = (ClampMin = "0", ClampMax = "180"))
	float MaxAngle = 35.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Assist", meta = (ClampMin = "0"))
	float Distance = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Assist", meta = (ClampMin = "0"))
	float RotationRate = 720.f;

	bool IsValid() const
	{
		return FMath::IsFinite(MaxAngle) && MaxAngle >= 0.f && MaxAngle <= 180.f
			&& FMath::IsFinite(Distance) && Distance >= 0.f
			&& FMath::IsFinite(RotationRate) && RotationRate >= 0.f;
	}
	bool IsEligible(const FVector& Offset, float IntentYaw) const
	{
		return IsValid() && Distance > 0.f && RotationRate > 0.f && FMath::IsFinite(IntentYaw)
			&& !Offset.ContainsNaN() && !Offset.GetSafeNormal2D().IsNearlyZero() && Offset.SizeSquared() <= FMath::Square(Distance)
			&& FMath::Abs(FMath::FindDeltaAngleDegrees(IntentYaw, static_cast<float>(Offset.Rotation().Yaw))) <= MaxAngle + KINDA_SMALL_NUMBER;
	}
	float StepYaw(float CurrentYaw, float TargetYaw, float IntentYaw, float HeroDelta) const
	{
		if (!IsValid() || !FMath::IsFinite(HeroDelta) || HeroDelta <= 0.f
			|| !FMath::IsFinite(CurrentYaw) || !FMath::IsFinite(TargetYaw) || !FMath::IsFinite(IntentYaw)) { return CurrentYaw; }
		const float DesiredOffset = FMath::Clamp(FMath::FindDeltaAngleDegrees(IntentYaw, TargetYaw), -MaxAngle, MaxAngle);
		const float Delta = FMath::Clamp(FMath::FindDeltaAngleDegrees(CurrentYaw, IntentYaw + DesiredOffset),
			-RotationRate * HeroDelta, RotationRate * HeroDelta);
		return FRotator::NormalizeAxis(CurrentYaw + Delta);
	}
};

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

/** Held frontal guard (R-CMB-22/23/49): blocked force becomes stamina damage; 0 stamina breaks into shared Staggered. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FHeroBlockData
{
	GENERATED_BODY()
	/** Fraction of a frontal hit's damage removed while blocking. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0", ClampMax = "1"))
	float DamageReduction = 0.8f;
	/** Stamina lost per point of the hit's original (unreduced) damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0"))
	float StaminaPerDamage = 1.f;
	/** Total width of the guarded front arc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0", ClampMax = "360"))
	float ArcDegrees = 140.f;
	/** Blocking regeneration waits this long after every absorbed hit (on top of the normal regen delay). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0"))
	float BlockRegenSuppressAfterHit = 0.6f;
	/** Shared Staggered duration applied to the hero on block break. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0.01"))
	float BlockBreakStaggerDuration = 1.2f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0.01", ClampMax = "1"))
	float MoveSpeedMultiplier = 0.5f;
	/** Presentation only: plays over the guard without leaving Block. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UAnimMontage> BlockHitMontage;
	/** Presentation only: plays while the shared Staggered state gates actions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	TObjectPtr<UAnimMontage> BlockBreakMontage;
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
	/** Left/Right montages are forward-moving placeholders: turn toward the input before playing them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	bool bSideClipsFaceInput = false;
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
	/** bFacingFixed: the hero keeps its facing (camera-facing or locked-on), so the clip follows input relative to it.
	 *  Otherwise the hero turns toward the input and dodges forward. */
	static EHeroDodgeDirection SelectDirection(const FVector& WorldInput, const FVector& Facing, bool bFacingFixed)
	{
		if (WorldInput.IsNearlyZero()) { return EHeroDodgeDirection::Backward; }
		if (!bFacingFixed) { return EHeroDodgeDirection::Forward; }
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

	/** True: the hero turns toward the camera yaw and strafes/backpedals. False: it turns toward its movement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bFaceCameraDirection = true;
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
