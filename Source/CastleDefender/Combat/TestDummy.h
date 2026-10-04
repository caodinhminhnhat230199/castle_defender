#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "TestDummy.generated.h"

class UHealthComponent;
class UStaticMeshComponent;
struct FCombatHit;

/** Debug target with health and a team. Spawned by the SpawnTestDummy cheat and used by functional tests. */
UCLASS()
class CASTLEDEFENDER_API ATestDummy : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ATestDummy();

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { TeamId = NewTeamId; }
	virtual void Tick(float DeltaSeconds) override;

	/** Test entry point. Switch to UCombatLibrary::DeliverHit when T-CMB-04 lands. */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ApplyDebugHit(float Damage);

	UFUNCTION(BlueprintPure, Category = "Debug")
	UHealthComponent* GetHealth() const { return Health; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FGenericTeamId TeamId;

private:
	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit);
};
