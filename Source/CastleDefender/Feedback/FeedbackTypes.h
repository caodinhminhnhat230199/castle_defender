#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "FeedbackTypes.generated.h"

class UCameraShakeBase;
class UNiagaraSystem;
class USoundBase;
class UTexture2D;

/** One DT_Feedback row: everything a Feedback.* event plays (R-UXF-02). Row name must equal Tag. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FFeedbackRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (Categories = "Feedback"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	/** Impact rows: sound per physical surface of the target; falls back to Sound. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> SurfaceSounds;

	/** Global alerts play 2D; everything else at the context location. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	bool bSound2D = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> Niagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	bool bAttachToTarget = false;

	/** Played from T-UXF-03. Radius 0 = direct shake, Hero-involved only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0"))
	float ShakeScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0", Units = "cm"))
	float ShakeInnerRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0", Units = "cm"))
	float ShakeOuterRadius = 0.f;

	/** Played from T-UXF-03. 0 = no hit stop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitStop", meta = (ClampMin = "0", Units = "s"))
	float HitStopSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitStop")
	bool bHeroOnly = false;

	/** Empty = no toast. Args: {Detail}, {Lane}. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toast")
	FText ToastText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Toast")
	TSoftObjectPtr<UTexture2D> ToastIcon;

	/** R-UXF-03a [TUNABLE]: minimum seconds between plays (per tag, or per tag and actor). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throttle", meta = (ClampMin = "0", Units = "s"))
	float CooldownSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throttle")
	bool bCooldownPerActor = false;

	/** R-UXF-03b [TUNABLE]: max plays per BurstWindow. 0 = Game Tuning default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throttle", meta = (ClampMin = "0"))
	int32 BurstLimit = 0;

	/** 0 = Game Tuning default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throttle", meta = (ClampMin = "0", Units = "s"))
	float BurstWindow = 0.f;
};

/** What the caller knows about the event. Filled by the gameplay owner, read only by presentation. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FFeedbackEventContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	bool bIsHeavy = false;

	/** Picks the ".Armored" variant row when no explicit Variant is set (R-UXF-12). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	bool bTargetArmored = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	TEnumAsByte<EPhysicalSurface> Surface = SurfaceType_Default;

	/** Last tag segment of a variant row, e.g. "Infantry" for <Tag>.Infantry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FName Variant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FName Lane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	float Magnitude = 1.f;

	/** Squad type, perk asset name, wave index... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FName Detail;

	/** Zero = use the target's location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
	FVector Direction = FVector::ZeroVector;
};

/** World-level HUD layers (R-UXF-23). Flags; the highest set layer drives visibility; none set = Combat. */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EHUDLayer : uint8
{
	None = 0 UMETA(Hidden),
	CommandWheel = 1 << 0,
	Build = 1 << 1,
	TacticalFocus = 1 << 2,
	CommanderSpirit = 1 << 3,
	Modal = 1 << 4,
};
ENUM_CLASS_FLAGS(EHUDLayer);

/** Cooldown and burst limit per tag (R-UXF-03). Plain struct with explicit time so specs drive it directly. */
struct CASTLEDEFENDER_API FFeedbackThrottle
{
	/** True and records the play when allowed. ActorKey is used only when bPerActor. */
	bool TryPlay(FGameplayTag Tag, const UObject* ActorKey, double Now,
		float CooldownSeconds, bool bPerActor, int32 BurstLimit, float BurstWindow);

	void Reset();

private:
	/** (tag, actor or null) -> time the cooldown ends. */
	TMap<TPair<FGameplayTag, FObjectKey>, double> CooldownEnds;
	/** Play times inside the current burst window, oldest first. */
	TMap<FGameplayTag, TArray<double>> BurstPlays;
};

/** Tag and (tag, variant) lookup over a DT_Feedback table (R-UXF-02). */
struct CASTLEDEFENDER_API FFeedbackRowIndex
{
	/** Rebuilds from the table. Returns warnings for rows with an invalid tag or a name that differs from the tag. */
	TArray<FString> Build(const UDataTable* Table);

	/** Variant row if present, else the base row, else null. Empty Variant + bTargetArmored = "Armored". */
	const FFeedbackRow* Find(FGameplayTag Tag, FName Variant, bool bTargetArmored) const;

	int32 Num() const { return Rows.Num(); }

private:
	TMap<FGameplayTag, const FFeedbackRow*> Rows;
	TMap<TPair<FGameplayTag, FName>, const FFeedbackRow*> Variants;
};
