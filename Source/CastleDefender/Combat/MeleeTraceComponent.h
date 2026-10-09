#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTypes.h"
#include "MeleeTraceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitResolvedSignature, AActor*, Target, ECombatHitResult, Result);

/**
 * Sweeps along weapon blade sockets (base, mid, tip) during active combat hit windows (T-CMB-04).
 * Implements the one-hit-per-target-per-swing guarantee (AC-CMB-01) by tracking AlreadyHitActors.
 * Reusable by Hero, Enemies (ENM), and Squad Soldiers (SQD).
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class CASTLEDEFENDER_API UMeleeTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeTraceComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Configures the attack payload and trace geometry for upcoming hit windows. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetPendingAttack(const FCombatHit& Template, float Radius = 25.f, FName StartSocket = FName(TEXT("weapon_base")), FName EndSocket = FName(TEXT("weapon_tip")), int32 SamplePoints = 3);

	/** Begins the active hit window: resets already-hit set and activates component tick. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginHitWindow();

	/** Ends the active hit window: disables tick and clears tracking. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndHitWindow();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsHitWindowActive() const { return bHitWindowActive; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetTraceMesh(USceneComponent* InMesh) { TraceMeshComponent = InMesh; }

	const TSet<TWeakObjectPtr<AActor>>& GetAlreadyHitActors() const { return AlreadyHitActors; }
	const FCombatHit& GetPendingAttackTemplate() const { return PendingHitTemplate; }

	/** Native: fires when a hit window opens. Enemies stop wind-up tracking here (T-ENM-03). */
	FSimpleMulticastDelegate OnHitWindowBegin;
	/** R-CMB-27: consume one counter payload before delivery can cause reentrant hits. */
	FSimpleMulticastDelegate OnParryCounterConsumed;
	void SetParryCounterMultiplier(float Multiplier);

	/** Broadcast when a hit attempt on a target completes through DeliverHit. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitResolvedSignature OnHitResolved;

	/** Attempts to deliver a hit to a specific candidate actor during active hit window (AC-CMB-01). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryHitTarget(AActor* HitActor, const FVector& ImpactPoint = FVector::ZeroVector, EPhysicalSurface Surface = SurfaceType_Default);

	/** Performs a manual sweep iteration between two sets of sample positions. */
	void ProcessSweepStep(const TArray<FVector>& PreviousPositions, const TArray<FVector>& CurrentPositions);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	float TraceRadius = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	FName SocketStart = FName(TEXT("weapon_base"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace")
	FName SocketEnd = FName(TEXT("weapon_tip"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trace", meta = (ClampMin = "1", ClampMax = "10"))
	int32 NumberOfSamplePoints = 3;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Trace")
	bool bHitWindowActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FCombatHit PendingHitTemplate;

private:
	TArray<FVector> ComputeSamplePositions() const;

	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
	TArray<FVector> PreviousSamplePositions;
	float CounterBaseDamage = 0.f;
	uint32 HitWindowGeneration = 0;

	UPROPERTY()
	TWeakObjectPtr<USceneComponent> TraceMeshComponent;
};
