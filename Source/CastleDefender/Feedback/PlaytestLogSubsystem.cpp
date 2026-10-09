#include "Feedback/PlaytestLogSubsystem.h"

#if !UE_BUILD_SHIPPING
#include "Feedback/FeedbackSubsystem.h"
#include "Core/GameLog.h"
#include "Core/GameTuningSettings.h"
#include "Combat/HealthComponent.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroCombatComponent.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"

namespace
{
	TAutoConsoleVariable<int32> PlaytestLog(TEXT("game.playtest.Log"), 1, TEXT("Record local development session telemetry in Saved/Playtest."), ECVF_Cheat);
	bool IsReserved(FName Key) { return Key == TEXT("ev") || Key == TEXT("t") || Key == TEXT("rt"); }
}
#endif

bool UPlaytestLogSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return Super::ShouldCreateSubsystem(Outer);
#endif
}
bool UPlaytestLogSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
void UPlaytestLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
	Collection.InitializeDependency<UFeedbackSubsystem>();
#endif
}
void UPlaytestLogSubsystem::OnWorldBeginPlay(UWorld& World)
{
	Super::OnWorldBeginPlay(World);
#if !UE_BUILD_SHIPPING
	// Isolated automation worlds have no game instance and are not playable recording sessions.
	if (!World.GetGameInstance()) { return; }
	bSessionActive = true;
	StartGameTime = World.GetTimeSeconds();
	Feedback = World.GetSubsystem<UFeedbackSubsystem>();
	Feedback->OnFeedbackPlayed.AddDynamic(this, &UPlaytestLogSubsystem::HandleFeedback);
	for (TActorIterator<APlayerController> It(&World); It; ++It) { BindController(*It); }
	SpawnHandle = World.AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [this](AActor* Actor)
	{
		if (APlayerController* Controller = Cast<APlayerController>(Actor)) { BindController(Controller); }
	}));
	if (IsRecording()) { OpenFile(); }
#endif
}
void UPlaytestLogSubsystem::LogEventForWorld(const UObject* WorldContextObject, FName Event, const TMap<FName, float>& Numbers, const TMap<FName, FString>& Strings)
{
#if !UE_BUILD_SHIPPING
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (World) { if (UPlaytestLogSubsystem* Log = World->GetSubsystem<UPlaytestLogSubsystem>()) { Log->LogEvent(Event, Numbers, Strings); } }
#endif
}
FString UPlaytestLogSubsystem::GetLogPath() const
{
#if !UE_BUILD_SHIPPING
	return LogPath;
#else
	return FString();
#endif
}
void UPlaytestLogSubsystem::LogEvent(FName Event, const TMap<FName, float>& Numbers, const TMap<FName, FString>& Strings)
{
#if !UE_BUILD_SHIPPING
	if (Event.IsNone() || !IsRecording() || !OpenFile()) { return; }
	WriteLine(Event, [&](FPlaytestJsonWriter& Json)
	{
		for (const auto& Pair : Numbers)
		{
			if (IsReserved(Pair.Key)) { continue; }
			if (FMath::IsFinite(Pair.Value)) { Json.WriteValue(Pair.Key.ToString(), Pair.Value); }
			else { Json.WriteNull(Pair.Key.ToString()); }
		}
		for (const auto& Pair : Strings)
		{
			if (!IsReserved(Pair.Key) && !Numbers.Contains(Pair.Key)) { Json.WriteValue(Pair.Key.ToString(), Pair.Value); }
		}
	});
#endif
}
void UPlaytestLogSubsystem::HandleFeedback(FGameplayTag Tag, const FFeedbackEventContext& Context)
{
#if !UE_BUILD_SHIPPING
	if (!IsRecording() || !OpenFile()) { return; }
	Summary.RecordFeedback(Tag);
	WriteLine(TEXT("fb"), [&](FPlaytestJsonWriter& Json)
	{
		Json.WriteValue(TEXT("tag"), Tag.ToString());
		Json.WriteValue(TEXT("inst"), Context.Instigator.IsValid() ? Context.Instigator->GetClass()->GetName() : TEXT("None"));
		Json.WriteValue(TEXT("tgt"), Context.Target.IsValid() ? Context.Target->GetClass()->GetName() : TEXT("None"));
		Json.WriteValue(TEXT("d"), Context.Detail.ToString());
		Json.WriteValue(TEXT("lane"), Context.Lane.ToString());
		Json.WriteArrayStart(TEXT("loc"));
		for (double Value : { Context.Location.X, Context.Location.Y, Context.Location.Z })
		{
			if (FMath::IsFinite(Value)) { Json.WriteValue(Value); }
			else { Json.WriteNull(); }
		}
		Json.WriteArrayEnd();
	});
#endif
}
void UPlaytestLogSubsystem::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
#if !UE_BUILD_SHIPPING
	BindHero(Cast<AHeroCharacter>(NewPawn));
#endif
}
void UPlaytestLogSubsystem::HandleAction(EHeroActionState OldState, EHeroActionState NewState)
{
#if !UE_BUILD_SHIPPING
	if (!IsRecording() || !OpenFile()) { return; }
	FName Action;
	switch (NewState)
	{
	case EHeroActionState::LightAttack: Action = TEXT("Light"); break;
	case EHeroActionState::HeavyAttack: Action = TEXT("Heavy"); break;
	case EHeroActionState::Dodge: Action = TEXT("Dodge"); break;
	case EHeroActionState::Block: Action = TEXT("Block"); break;
	case EHeroActionState::Parry: Action = TEXT("Parry"); break;
	default: return;
	}
	Summary.RecordAction(Action);
	LogEvent(TEXT("hero_action"), {}, { {TEXT("action"), Action.ToString()} });
#endif
}
void UPlaytestLogSubsystem::HandleDamaged(const FCombatHit& Hit, float NewHealth)
{
#if !UE_BUILD_SHIPPING
	// OnPossess can precede initialization. Before the first damage event the owner's MaxHealth is authoritative.
	if (LastHealth <= 0.f && ObservedHero.IsValid()) { LastHealth = ObservedHero->GetHealthComponent()->GetMaxHealth(); }
	const float Amount = FMath::Max(0.f, LastHealth - NewHealth);
	LastHealth = NewHealth;
	LogEvent(TEXT("hero_damaged"), { {TEXT("amount"), Amount}, {TEXT("hp"), NewHealth} }, {});
#endif
}
#if !UE_BUILD_SHIPPING
bool UPlaytestLogSubsystem::IsRecording() const { return bSessionActive && !bWriteFailed && PlaytestLog.GetValueOnGameThread() != 0; }
void UPlaytestLogSubsystem::BindController(APlayerController* Controller)
{
	if (ObservedController.IsValid()) { return; } // Single-player: one possessed Hero stream per world.
	ObservedController = Controller;
	Controller->OnPossessedPawnChanged.AddDynamic(this, &UPlaytestLogSubsystem::HandlePawnChanged);
	BindHero(Cast<AHeroCharacter>(Controller->GetPawn()));
}
void UPlaytestLogSubsystem::BindHero(AHeroCharacter* Hero)
{
	if (ObservedHero.IsValid())
	{
		ObservedHero->GetCombatComponent()->OnActionStateChanged.RemoveDynamic(this, &UPlaytestLogSubsystem::HandleAction);
		ObservedHero->GetHealthComponent()->OnDamaged.RemoveDynamic(this, &UPlaytestLogSubsystem::HandleDamaged);
	}
	ObservedHero = Hero;
	LastHealth = Hero ? Hero->GetHealthComponent()->GetCurrentHealth() : 0.f;
	if (Hero)
	{
		Hero->GetCombatComponent()->OnActionStateChanged.AddDynamic(this, &UPlaytestLogSubsystem::HandleAction);
		Hero->GetHealthComponent()->OnDamaged.AddDynamic(this, &UPlaytestLogSubsystem::HandleDamaged);
	}
}
bool UPlaytestLogSubsystem::OpenFile()
{
	if (Writer) { return true; }
	if (bWriteFailed) { return false; }
	const FString Folder = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Playtest"));
	if (!IFileManager::Get().MakeDirectory(*Folder, true)) { FailWrite(); return false; }
	const FString Base = Folder / (FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")) + TEXT("_") + FPaths::MakeValidFileName(GetWorld()->GetMapName()));
	LogPath = Base + TEXT(".jsonl");
	for (int32 Suffix = 2; IFileManager::Get().FileExists(*LogPath); ++Suffix) { LogPath = FString::Printf(TEXT("%s_%d.jsonl"), *Base, Suffix); }
	Writer.Reset(IFileManager::Get().CreateFileWriter(*LogPath, FILEWRITE_Append | FILEWRITE_AllowRead));
	if (!Writer) { FailWrite(); return false; }
	WriteLine(TEXT("session_start"), [&](FPlaytestJsonWriter& Json)
	{
		Json.WriteValue(TEXT("map"), GetWorld()->GetMapName());
		Json.WriteValue(TEXT("build"), FApp::GetBuildVersion());
		Json.WriteValue(TEXT("engine"), FEngineVersion::Current().ToString());
		Json.WriteValue(TEXT("date"), FDateTime::UtcNow().ToIso8601());
		const float Scale = UGameTuningSettings::Get()->CameraShakeScale;
		if (FMath::IsFinite(Scale)) { Json.WriteValue(TEXT("shake_scale"), Scale); }
		else { Json.WriteNull(TEXT("shake_scale")); }
	});
	return Writer.IsValid();
}
void UPlaytestLogSubsystem::WriteLine(FName Event, TFunctionRef<void(FPlaytestJsonWriter&)> WritePayload)
{
	if (!Writer) { return; }
	FString Line;
	const auto JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue(TEXT("ev"), Event.ToString());
	JsonWriter->WriteValue(TEXT("t"), GetWorld()->GetTimeSeconds());
	JsonWriter->WriteValue(TEXT("rt"), GetWorld()->GetRealTimeSeconds());
	WritePayload(*JsonWriter);
	JsonWriter->WriteObjectEnd();
	if (!JsonWriter->Close()) { FailWrite(); return; }
	Line += TEXT("\n");
	FTCHARToUTF8 Utf8(*Line);
	Writer->Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Utf8.Length());
	Writer->Flush();
	if (Writer->IsError()) { FailWrite(); }
}
void UPlaytestLogSubsystem::FailWrite()
{
	if (!bWriteFailed) { UE_LOG(LogGameFeedback, Warning, TEXT("Playtest telemetry write failed; logging disabled for this session.")); }
	bWriteFailed = true;
	Writer.Reset();
}
#endif
void UPlaytestLogSubsystem::Deinitialize()
{
#if !UE_BUILD_SHIPPING
	if (Feedback.IsValid()) { Feedback->OnFeedbackPlayed.RemoveDynamic(this, &UPlaytestLogSubsystem::HandleFeedback); }
	if (ObservedController.IsValid()) { ObservedController->OnPossessedPawnChanged.RemoveDynamic(this, &UPlaytestLogSubsystem::HandlePawnChanged); }
	GetWorld()->RemoveOnActorSpawnedHandler(SpawnHandle);
	BindHero(nullptr);
	if (Writer) { WriteLine(TEXT("summary"), [&](FPlaytestJsonWriter& Json) { Summary.WriteFields(Json, GetWorld()->GetTimeSeconds() - StartGameTime); }); }
	Writer.Reset();
	bSessionActive = false;
#endif
	Super::Deinitialize();
}
