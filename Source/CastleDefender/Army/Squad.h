#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "Army/SquadTypes.h"
#include "Squad.generated.h"

class USquadDefinition;
class UCommandComponent;

/** Level-owned squad anchor (T-SQD-01); movement and orders arrive in later SQD tasks. */
UCLASS()
class CASTLEDEFENDER_API ASquad : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:
	ASquad();
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UFUNCTION(BlueprintPure, Category = "Squad")
	TArray<ASoldierCharacter*> GetSoldiers() const;
	UFUNCTION(BlueprintPure, Category = "Squad")
	FTransform GetHomeTransform() const { return HomeTransform; }
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Squad", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<USquadDefinition> Definition;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void ReleaseSoldiers();
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Squad", meta = (AllowPrivateAccess = "true"))
	FTransform HomeTransform;
	UPROPERTY()
	TArray<FSoldierRuntime> Soldiers;
	UPROPERTY()
	TWeakObjectPtr<UCommandComponent> Registry;
};
