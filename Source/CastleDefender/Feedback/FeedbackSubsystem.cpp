#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"

#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Combat/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/GameTuningSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"
#include "Kismet/GameplayStatics.h"
// UE 5.8 Niagara headers still override the deprecated GetAssetRegistryTags (engine-side C4996).
PRAGMA_DISABLE_DEPRECATION_WARNINGS
#include "NiagaraFunctionLibrary.h"
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#include "Sound/SoundBase.h"
#include "Hero/HeroCharacter.h"
#include "HAL/PlatformTime.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"

namespace
{
	constexpr int32 MaxRecentTags = 10;

#if !UE_BUILD_SHIPPING
	FAutoConsoleCommand CoverageCommand(
		TEXT("game.feedback.Coverage"),
		TEXT("Lists declared Feedback.* tags that no row has played in this world."),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda(
			[](const TArray<FString>&, UWorld* World, FOutputDevice& Ar)
			{
				const UFeedbackSubsystem* Feedback = UFeedbackSubsystem::Get(World);
				if (!Feedback)
				{
					Ar.Log(TEXT("game.feedback.Coverage: no feedback subsystem in this world (game or PIE only)."));
					return;
				}
				const TArray<FGameplayTag> Unplayed = Feedback->GetUnplayedDeclaredTags();
				Ar.Logf(TEXT("game.feedback.Coverage: %d declared Feedback tags not played:"), Unplayed.Num());
				for (const FGameplayTag& Tag : Unplayed)
				{
					Ar.Logf(TEXT("  %s"), *Tag.ToString());
				}
			}));
#endif
}

UFeedbackSubsystem* UFeedbackSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFeedbackSubsystem>() : nullptr;
}

bool UFeedbackSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFeedbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Small table in the prototype: loaded synchronously at map start (technical plan §3.6).
	SetFeedbackTable(UGameTuningSettings::Get()->FeedbackTable.LoadSynchronous());
}

void UFeedbackSubsystem::Deinitialize()
{
	ClearAfterimages();
	RestoreHitStops();
	StopCameraShakes();
	SetFeedbackTable(nullptr);
	Super::Deinitialize();
}

void UFeedbackSubsystem::SetFeedbackTable(UDataTable* Table)
{
#if WITH_EDITOR
	if (FeedbackTable) { FeedbackTable->OnDataTableChanged().RemoveAll(this); }
	if (Table) { Table->OnDataTableChanged().AddUObject(this, &UFeedbackSubsystem::RebuildIndex); }
#endif
	FeedbackTable = Table;
	Throttle.Reset();
	PlayedTags.Reset();
	RecentTags.Reset();
	RebuildIndex();
}

void UFeedbackSubsystem::RebuildIndex()
{
	// Row pointers live in the table; rebuild whenever it changes (editor edits during PIE included).
	for (const FString& Warning : RowIndex.Build(FeedbackTable))
	{
		UE_LOG(LogGameFeedback, Warning, TEXT("%s"), *Warning);
	}
}

bool UFeedbackSubsystem::Play(FGameplayTag Tag, const FFeedbackEventContext& Context)
{
	if (!Tag.IsValid())
	{
		WarnOnce(NAME_None, TEXT("Feedback Play called with an invalid tag."));
		return false;
	}
	if (!FeedbackTable)
	{
		WarnOnce(TEXT("FeedbackTable"), TEXT("Game Tuning FeedbackTable is unset or failed to load; feedback is disabled."));
		return false;
	}
	const FFeedbackRow* Row = RowIndex.Find(Tag, Context.Variant, Context.bTargetArmored);
	if (!Row)
	{
		// R-UXF-06: reported once per tag, then a no-op.
		WarnOnce(Tag.GetTagName(), FString::Printf(TEXT("No DT_Feedback row for %s."), *Tag.ToString()));
		return false;
	}

	const UGameTuningSettings* Settings = UGameTuningSettings::Get();
	const int32 BurstLimit = Row->BurstLimit > 0 ? Row->BurstLimit : Settings->DefaultBurstLimit;
	const float BurstWindow = Row->BurstWindow > 0.f ? Row->BurstWindow : Settings->DefaultBurstWindow;
	const AActor* ActorKey = Context.Target.IsValid() ? Context.Target.Get() : Context.Instigator.Get();
	// Spam is judged in real time, so Tactical Focus slow motion does not change it (D-20: presentation clock).
	if (!Throttle.TryPlay(Tag, ActorKey, GetWorld()->GetRealTimeSeconds(), Row->CooldownSeconds,
		Row->bCooldownPerActor, BurstLimit, BurstWindow))
	{
		return false;
	}

	PlayedTags.Add(Row->Tag);
	PlayOutputs(*Row, Context);

	RecentTags.Insert(Row->Tag, 0);
	RecentTags.SetNum(FMath::Min(RecentTags.Num(), MaxRecentTags));
	if (GameDebug::CVarFeedback.GetValueOnGameThread() != 0)
	{
		ShowRecentTags();
	}

	OnFeedbackPlayed.Broadcast(Row->Tag, Context);
	return true;
}

void UFeedbackSubsystem::PlayOutputs(const FFeedbackRow& Row, const FFeedbackEventContext& Context)
{
	AActor* Target = Context.Target.Get();
	const FVector Location = !Context.Location.IsZero() ? Context.Location
		: (Target ? Target->GetActorLocation() : FVector::ZeroVector);

	USoundBase* Sound = Row.Sound;
	if (const TObjectPtr<USoundBase>* SurfaceSound = Row.SurfaceSounds.Find(Context.Surface); SurfaceSound && *SurfaceSound)
	{
		Sound = *SurfaceSound;
	}
	if (Sound)
	{
		if (Row.bSound2D)
		{
			UGameplayStatics::PlaySound2D(this, Sound);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
		}
	}

	if (Row.Niagara)
	{
		if (Row.bAttachToTarget && Target && Target->GetRootComponent())
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(Row.Niagara, Target->GetRootComponent(), NAME_None,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}
		else
		{
			const FRotator Rotation = Context.Direction.IsNearlyZero() ? FRotator::ZeroRotator : Context.Direction.Rotation();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Row.Niagara, Location, Rotation);
		}
	}
	ApplyHitStop(Row, Context);
	SpawnAfterimage(Row, Context);
	ApplyCameraShake(Row, Context, Location);
}

void UFeedbackSubsystem::SpawnAfterimage(const FFeedbackRow& Row, const FFeedbackEventContext& Context)
{
	AActor* Target = Context.Target.Get();
	USkeletalMeshComponent* Source = Target ? Target->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	if (!Source || !Source->GetSkinnedAsset() || !Row.AfterimageMaterial
		|| !FMath::IsFinite(Row.AfterimageSeconds) || Row.AfterimageSeconds <= 0.f
		|| !FMath::IsFinite(Row.AfterimageOpacity) || Row.AfterimageOpacity <= 0.f) { return; }
	Afterimages.RemoveAll([](const FAfterimage& Entry) { return !Entry.Mesh.IsValid(); });
	UPoseableMeshComponent* Ghost = NewObject<UPoseableMeshComponent>(Target, NAME_None, RF_Transient);
	Target->AddInstanceComponent(Ghost);
	Ghost->SetSkinnedAssetAndUpdate(Source->GetSkinnedAsset());
	Ghost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ghost->SetGenerateOverlapEvents(false);
	Ghost->SetCanEverAffectNavigation(false);
	Ghost->SetCastShadow(false);
	Ghost->SetWorldTransform(Source->GetComponentTransform());
	Ghost->RegisterComponent();
	Ghost->CopyPoseFromSkeletalComponent(Source);
	Ghost->RefreshBoneTransforms();
	Ghost->SetComponentTickEnabled(false);
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Row.AfterimageMaterial, Ghost);
	for (int32 Index = 0; Index < Ghost->GetNumMaterials(); ++Index) { Ghost->SetMaterial(Index, Material); }
	const float Opacity = FMath::Clamp(Row.AfterimageOpacity, 0.f, 1.f);
	Material->SetScalarParameterValue(TEXT("GhostOpacity"), Opacity);
	const double Started = FPlatformTime::Seconds();
	const float Duration = Row.AfterimageSeconds;
	const TWeakObjectPtr<UPoseableMeshComponent> WeakGhost(Ghost);
	const TWeakObjectPtr<UMaterialInstanceDynamic> WeakMaterial(Material);
	// Real-time per-frame presentation fade is active only while a frozen afterimage exists (D-20).
	const auto Handle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this,
		[WeakGhost, WeakMaterial, Started, Duration, Opacity](float)
		{
			UPoseableMeshComponent* Mesh = WeakGhost.Get();
			if (!Mesh) { return false; }
			const AActor* Owner = Mesh->GetOwner();
			const UHealthComponent* Health = Owner ? Owner->FindComponentByClass<UHealthComponent>() : nullptr;
			const float Progress = (FPlatformTime::Seconds() - Started) / Duration;
			if (!IsValid(Owner) || (Health && Health->IsDead()) || Progress >= 1.f)
			{
				Mesh->DestroyComponent();
				return false;
			}
			if (UMaterialInstanceDynamic* MID = WeakMaterial.Get()) { MID->SetScalarParameterValue(TEXT("GhostOpacity"), Opacity * (1.f - Progress)); }
			return true;
		}));
	Afterimages.Add({ Ghost, Handle });
}

void UFeedbackSubsystem::ClearAfterimages()
{
	for (const FAfterimage& Entry : Afterimages)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(Entry.Handle);
		if (UPoseableMeshComponent* Mesh = Entry.Mesh.Get()) { Mesh->DestroyComponent(); }
	}
	Afterimages.Reset();
}

void UFeedbackSubsystem::ApplyCameraShake(const FFeedbackRow& Row, const FFeedbackEventContext& Context, const FVector& Location)
{
	if (!Row.CameraShake) { return; }
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APlayerCameraManager* Camera = PC && PC->IsLocalController() ? PC->PlayerCameraManager.Get() : nullptr;
	if (!Camera) { return; }
	if (FCameraShake* Previous = CameraShakes.Find(Row.Tag))
	{
		if (Previous->Camera.IsValid() && Previous->Instance.IsValid())
		{
			Previous->Camera->StopCameraShake(Previous->Instance.Get(), true);
		}
		CameraShakes.Remove(Row.Tag);
	}
	float Scale = Row.ShakeScale * UGameTuningSettings::Get()->CameraShakeScale;
	if (!FMath::IsFinite(Scale) || Scale <= 0.f) { return; }
	if (Row.ShakeOuterRadius <= 0.f && Row.ShakeInnerRadius <= 0.f)
	{
		const APawn* Hero = Cast<AHeroCharacter>(PC->GetPawn());
		if (!Hero || (Context.Instigator.Get() != Hero && Context.Target.Get() != Hero)) { return; }
	}
	else
	{
		const float Distance = FVector::Distance(Camera->GetCameraLocation(), Location);
		// Engine 5.8 radial falloff at exponent 1; PlayWorldCameraShake returns no instance or global scale.
		// Use the returned-instance API so R-UXF-10 can replace precisely this tag's shake.
		Scale *= Row.ShakeInnerRadius < Row.ShakeOuterRadius
			? 1.f - FMath::Clamp((Distance - Row.ShakeInnerRadius) / (Row.ShakeOuterRadius - Row.ShakeInnerRadius), 0.f, 1.f)
			: (Distance < Row.ShakeInnerRadius ? 1.f : 0.f);
	}
	if (Scale > 0.f)
	{
		if (UCameraShakeBase* Instance = Camera->StartCameraShake(Row.CameraShake, Scale))
		{
			CameraShakes.Add(Row.Tag, { Camera, Instance });
		}
	}
}

void UFeedbackSubsystem::StopCameraShakes()
{
	for (const auto& Pair : CameraShakes)
	{
		if (Pair.Value.Camera.IsValid() && Pair.Value.Instance.IsValid())
		{
			Pair.Value.Camera->StopCameraShake(Pair.Value.Instance.Get(), true);
		}
	}
	CameraShakes.Empty();
}

void UFeedbackSubsystem::ApplyHitStop(const FFeedbackRow& Row, const FFeedbackEventContext& Context)
{
	const bool bHeroInvolved = Cast<AHeroCharacter>(Context.Instigator.Get()) || Cast<AHeroCharacter>(Context.Target.Get());
	// R-UXF-07: a row cannot opt minor hits or non-Hero fights into hit stop.
	const bool bLargeImpact = Row.Tag.MatchesTag(FeedbackTags::Combat_Hit_Heavy) || Row.Tag == FeedbackTags::Combat_Parry ||
		Row.Tag == FeedbackTags::Combat_BlockBreak || Row.Tag == FeedbackTags::State_Staggered_Applied;
	const UGameTuningSettings* Settings = UGameTuningSettings::Get();
	if (!bLargeImpact || !bHeroInvolved || !FMath::IsFinite(Row.HitStopSeconds) || Row.HitStopSeconds <= 0.f ||
		!FMath::IsFinite(Settings->MaxHitStopSeconds) || Settings->MaxHitStopSeconds <= 0.f || !FMath::IsFinite(Settings->HitStopDilation)) { return; }
	const double EndTime = FPlatformTime::Seconds() + FMath::Min(Row.HitStopSeconds, Settings->MaxHitStopSeconds);
	const float Dilation = FMath::Clamp(Settings->HitStopDilation, 0.f, 1.f);
	StopActor(Context.Instigator.Get(), EndTime, Dilation);
	StopActor(Context.Target.Get(), EndTime, Dilation);
}

void UFeedbackSubsystem::StopActor(AActor* Actor, double EndTime, float Dilation)
{
	if (!IsValid(Actor) || Actor->IsActorBeingDestroyed()) { return; }
	const TWeakObjectPtr<AActor> WeakActor(Actor);
	if (FHitStop* Existing = HitStops.Find(WeakActor))
	{
		Existing->EndTime = FMath::Max(Existing->EndTime, EndTime);
		Actor->CustomTimeDilation = Dilation;
		return;
	}
	FHitStop& Stop = HitStops.Add(WeakActor);
	Stop.OriginalDilation = Actor->CustomTimeDilation;
	Stop.EndTime = EndTime;
	Actor->CustomTimeDilation = Dilation;
	const TWeakObjectPtr<UFeedbackSubsystem> WeakThis(this);
	// D-20: per-frame only while this actor is stopped; real-time restore ignores world/actor dilation.
	Stop.Handle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis, WeakActor](float)
	{
		UFeedbackSubsystem* Subsystem = WeakThis.Get();
		if (!Subsystem) { return false; }
		FHitStop* Pending = Subsystem->HitStops.Find(WeakActor);
		if (!Pending) { return false; }
		if (!WeakActor.IsValid()) { Subsystem->HitStops.Remove(WeakActor); return false; }
		if (FPlatformTime::Seconds() < Pending->EndTime) { return true; }
		WeakActor->CustomTimeDilation = Pending->OriginalDilation;
		Subsystem->HitStops.Remove(WeakActor);
		return false;
	}));
}

void UFeedbackSubsystem::RestoreHitStops()
{
	for (const auto& Pair : HitStops)
	{
		FTSTicker::GetCoreTicker().RemoveTicker(Pair.Value.Handle);
		if (AActor* Actor = Pair.Key.Get()) { Actor->CustomTimeDilation = Pair.Value.OriginalDilation; }
	}
	HitStops.Empty();
}

void UFeedbackSubsystem::SetHUDLayerActive(EHUDLayer Layer, bool bActive)
{
	const EHUDLayer Previous = HUDLayers;
	if (bActive)
	{
		EnumAddFlags(HUDLayers, Layer);
	}
	else
	{
		EnumRemoveFlags(HUDLayers, Layer);
	}
	if (HUDLayers != Previous)
	{
		OnHUDLayersChanged.Broadcast(static_cast<int32>(HUDLayers));
	}
}

EHUDLayer UFeedbackSubsystem::GetTopHUDLayer() const
{
	for (const EHUDLayer Layer : { EHUDLayer::Modal, EHUDLayer::CommanderSpirit, EHUDLayer::TacticalFocus,
		EHUDLayer::Build, EHUDLayer::CommandWheel })
	{
		if (EnumHasAnyFlags(HUDLayers, Layer))
		{
			return Layer;
		}
	}
	return EHUDLayer::None;
}

bool UFeedbackSubsystem::IsTacticalDisplay() const
{
	return EnumHasAnyFlags(HUDLayers, EHUDLayer::TacticalFocus | EHUDLayer::CommanderSpirit)
		&& !EnumHasAnyFlags(HUDLayers, EHUDLayer::Modal);
}

TArray<FGameplayTag> UFeedbackSubsystem::GetUnplayedDeclaredTags() const
{
	TArray<FGameplayTag> Unplayed;
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	for (const FGameplayTag& Tag : Manager.RequestGameplayTagChildren(GameTags::Feedback))
	{
		// Implicit parents such as Feedback.Combat.Hit are not events.
		const TSharedPtr<FGameplayTagNode> Node = Manager.FindTagNode(Tag);
		if (Node && Node->IsExplicitTag() && !PlayedTags.Contains(Tag))
		{
			Unplayed.Add(Tag);
		}
	}
	return Unplayed;
}

void UFeedbackSubsystem::WarnOnce(FName Key, const FString& Message)
{
	bool bAlreadyWarned = false;
	WarnedKeys.Add(Key, &bAlreadyWarned);
	if (bAlreadyWarned)
	{
		return;
	}
	UE_LOG(LogGameFeedback, Warning, TEXT("%s"), *Message);
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, Message);
	}
#endif
}

void UFeedbackSubsystem::ShowRecentTags() const
{
#if !UE_BUILD_SHIPPING
	if (!GEngine)
	{
		return;
	}
	FString Text = TEXT("Feedback (newest first):");
	for (const FGameplayTag& Tag : RecentTags)
	{
		Text += TEXT("\n  ") + Tag.ToString();
	}
	// One message per world, replaced on every play; no Tick needed.
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 10.f, FColor::Cyan, Text);
#endif
}
