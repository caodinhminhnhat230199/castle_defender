#include "Feedback/FeedbackSubsystem.h"

#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
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

void UFeedbackSubsystem::PlayOutputs(const FFeedbackRow& Row, const FFeedbackEventContext& Context) const
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
