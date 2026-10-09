#pragma once

#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "ImpactTestCamera.generated.h"

UCLASS(Transient)
class UImpactTestShake : public UCameraShakeBase
{
	GENERATED_BODY()
};

/** Records the Engine camera API boundary without requiring a rendered camera in automation worlds. */
UCLASS(Transient)
class AImpactTestCamera : public APlayerCameraManager
{
	GENERATED_BODY()
public:
	int32 Starts = 0;
	int32 Stops = 0;
	float LastScale = 0.f;
	TWeakObjectPtr<UCameraShakeBase> LastStarted;
	TWeakObjectPtr<UCameraShakeBase> LastStopped;

	virtual UCameraShakeBase* StartCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale,
		ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot) override
	{
		++Starts;
		LastScale = Scale;
		UCameraShakeBase* Shake = NewObject<UCameraShakeBase>(this, ShakeClass);
		LastStarted = Shake;
		return Shake;
	}
	virtual void StopCameraShake(UCameraShakeBase* ShakeInstance, bool bImmediately) override
	{
		++Stops;
		LastStopped = ShakeInstance;
	}
};
