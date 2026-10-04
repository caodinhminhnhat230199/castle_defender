#pragma once

#include "CoreMinimal.h"
#include "HeroCombatTypes.generated.h"

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
