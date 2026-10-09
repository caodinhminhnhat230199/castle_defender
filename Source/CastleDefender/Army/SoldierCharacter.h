#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Combat/CombatTypes.h"
#include "SoldierCharacter.generated.h"

class ASquad;
class USquadDefinition;
class UHealthComponent;
class UCombatStateComponent;

/** Bare soldier body; movement/avoidance and attack behavior are later SQD tasks. */
UCLASS()
class CASTLEDEFENDER_API ASoldierCharacter : public ACharacter, public IGenericTeamAgentInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:
	ASoldierCharacter();
	void InitFromSquad(ASquad* InSquad, int32 InSlotIndex, USquadDefinition* InDefinition);
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamId); }
	virtual void SetGenericTeamId(const FGenericTeamId& InTeam) override { TeamId = InTeam.GetId(); }
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UFUNCTION(BlueprintPure, Category = "Squad")
	ASquad* GetSquad() const;
	UFUNCTION(BlueprintPure, Category = "Squad")
	int32 GetSlotIndex() const { return SlotIndex; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	UHealthComponent* GetHealthComponent() const { return Health; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatStateComponent* GetCombatStateComponent() const { return CombatState; }
protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatStateComponent> CombatState;
private:
	UPROPERTY()
	TWeakObjectPtr<ASquad> Squad;
	UPROPERTY()
	TObjectPtr<USquadDefinition> Definition;
	int32 SlotIndex = INDEX_NONE;
	uint8 TeamId = Team_Player;
};
