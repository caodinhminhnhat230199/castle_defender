#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyCharacter.h"
#include "EnemyLifecycleTestListener.generated.h"

/** Dynamic lifecycle observer, including a reentrant removal callback. */
UCLASS(Transient)
class UEnemyLifecycleTestListener : public UObject
{
	GENERATED_BODY()
public:
	int32 Count = 0;
	EEnemyRemovedReason LastReason = EEnemyRemovedReason::Despawned;
	bool bDespawnOnRemoval = false;
	bool bSawTerminalState = false;
	UFUNCTION()
	void HandleRemoved(AEnemyCharacter* Enemy, EEnemyRemovedReason Reason)
	{
		++Count;
		LastReason = Reason;
		bSawTerminalState = Enemy->HasReportedRemoval();
		if (bDespawnOnRemoval) { Enemy->Despawn(); }
	}
};
