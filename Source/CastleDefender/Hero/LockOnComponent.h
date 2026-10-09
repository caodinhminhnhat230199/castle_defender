#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTypes.h"
#include "Hero/HeroCombatTypes.h"
#include "LockOnComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLockOnTargetChangedSignature, AActor*, Target);

/** Pawn-lifetime lock owner. Candidate scans occur only on acquire, switch or target removal. */
UCLASS(ClassGroup = (Hero), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	ULockOnComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void Toggle();
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void Switch(float Direction);
	UFUNCTION(BlueprintCallable, Category = "Lock On")
	void Release();
	UFUNCTION(BlueprintPure, Category = "Lock On")
	AActor* GetLockOnTarget() const { return Target.Get(); }
	UFUNCTION(BlueprintPure, Category = "Lock On")
	FVector GetTargetLocation() const;
	UPROPERTY(BlueprintAssignable, Category = "Lock On")
	FLockOnTargetChangedSignature OnLockOnTargetChanged;

	/** Low-rate world-game-time validation; also used by focused integration tests. */
	void ValidateTarget();
private:
	const FHeroLockOnData* GetData() const;
	bool CanLock() const;
	bool IsAliveHostile(AActor* Candidate) const;
	bool HasLOS(AActor* Candidate) const;
	FVector TargetLocation(AActor* Candidate) const;
	void GetView(FVector& Location, FRotator& Rotation) const;
	bool ProjectCandidate(AActor* Candidate, FVector2D& Screen) const;
	TArray<AActor*> GatherCandidates() const;
	AActor* FindBest(bool bNearest) const;
	void SetTarget(AActor* NewTarget);
	void Retarget();
	UFUNCTION()
	void HandleTargetDeath(const FCombatHit& Hit);
	UFUNCTION()
	void HandleTargetDestroyed(AActor* DestroyedActor);
	UFUNCTION()
	void HandleOwnerDeath(const FCombatHit& Hit);

	TWeakObjectPtr<AActor> Target;
	FTimerHandle ValidationTimer;
	double LOSLostAt = -1.0;
};
