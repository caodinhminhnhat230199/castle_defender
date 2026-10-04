#pragma once

#include "CoreMinimal.h"
#include "HeroCombatTypes.generated.h"

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
