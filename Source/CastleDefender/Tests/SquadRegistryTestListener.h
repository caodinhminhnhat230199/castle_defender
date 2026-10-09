#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Player/CommandComponent.h"
#include "SquadRegistryTestListener.generated.h"

UCLASS()
class USquadRegistryTestListener : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UCommandComponent> Registry;
	int32 Changes = 0;
	TArray<int32> Counts;
	UFUNCTION()
	void HandleChanged()
	{
		++Changes;
		Counts.Add(Registry ? Registry->GetSquads().Num() : INDEX_NONE);
	}
};
