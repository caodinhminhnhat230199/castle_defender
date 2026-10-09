#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommandComponent.generated.h"

class ASquad;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSquadsChangedSignature);

/** Controller-lifetime registry; command input/orders are T-SQD-04/05, not this skeleton. */
UCLASS()
class CASTLEDEFENDER_API UCommandComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCommandComponent();
	bool RegisterSquad(ASquad* Squad);
	void UnregisterSquad(ASquad* Squad);
	UFUNCTION(BlueprintPure, Category = "Squads")
	TArray<ASquad*> GetSquads() const;
	UPROPERTY(BlueprintAssignable, Category = "Squads")
	FSquadsChangedSignature OnSquadsChanged;
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ASquad>> Squads;
};
