#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "CombatStateTestListener.generated.h"

/** Test helper: counts UCombatStateComponent broadcasts (dynamic delegates need a UFUNCTION target). */
UCLASS(Transient)
class UCombatStateTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 AddedCount = 0;
	int32 RemovedCount = 0;
	AActor* LastInstigator = nullptr;

	UFUNCTION()
	void HandleAdded(FGameplayTag State, AActor* Instigator) { ++AddedCount; LastInstigator = Instigator; }

	UFUNCTION()
	void HandleRemoved(FGameplayTag State) { ++RemovedCount; }
};
