#include "Army/Squad.h"
#include "Army/SquadDefinition.h"
#include "Army/SoldierCharacter.h"
#include "Components/SceneComponent.h"
#include "Player/HeroPlayerController.h"
#include "Player/CommandComponent.h"
#include "Core/GameLog.h"
#include "Engine/World.h"
#if !UE_BUILD_SHIPPING
#include "Core/GameDebug.h"
#include "Debug/DebugDrawService.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

namespace SquadSpawnDebug
{
	// Debug-only per-frame work: anchor and slot labels follow moving actors while the overlay is enabled.
	void Draw(UCanvas* Canvas, APlayerController* Viewer)
	{
		if (!Canvas || !Viewer || (GameDebug::CVarSquads.GetValueOnGameThread() == 0 && GameDebug::CVarArmy.GetValueOnGameThread() == 0)) { return; }
		for (TActorIterator<ASquad> It(Viewer->GetWorld()); It; ++It)
		{
			const TArray<ASoldierCharacter*> Soldiers = It->GetSoldiers();
			if (Soldiers.IsEmpty()) { continue; }
			DrawDebugSphere(Viewer->GetWorld(), It->GetActorLocation(), 30.f, 12, FColor::Cyan, false, 0.f);
			for (const ASoldierCharacter* Soldier : Soldiers)
			{
				const FVector Screen = Canvas->Project(Soldier->GetActorLocation() + FVector(0, 0, 120));
				if (Screen.Z > 0.f)
				{
					Canvas->SetDrawColor(FColor::Cyan);
					Canvas->DrawText(GEngine->GetSmallFont(), FString::Printf(TEXT("%s / slot %d"), *It->GetName(), Soldier->GetSlotIndex() + 1), Screen.X, Screen.Y);
				}
			}
		}
	}
}
#endif

ASquad::ASquad()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Anchor")));
}

void ASquad::BeginPlay()
{
	HomeTransform = GetActorTransform();
	Super::BeginPlay();
	if (IsActorBeingDestroyed()) { return; }
	FString Error;
	if (!Definition || !Definition->ValidateDefinition(Error))
	{
		UE_LOG(LogGameArmy, Error, TEXT("%s: invalid squad definition: %s"), *GetName(), Definition ? *Error : TEXT("missing definition"));
		return;
	}
	AHeroPlayerController* Controller = Cast<AHeroPlayerController>(GetWorld()->GetFirstPlayerController());
	Registry = Controller && Controller->IsLocalController() ? Controller->GetCommandComponent() : nullptr;
	// R-SQD-01: reserve a registry slot before creating any soldiers.
	if (!Registry.IsValid() || !Registry->RegisterSquad(this))
	{
		UE_LOG(LogGameArmy, Error, TEXT("%s: cannot register squad (missing local controller or active squad cap). No soldiers spawned."), *GetName());
		return;
	}
	if (IsActorBeingDestroyed() || !Registry.IsValid() || !Registry->GetSquads().Contains(this)) { return; }
	const int32 Count = Definition->SoldierCount;
	const int32 Columns = FMath::Min(Definition->FormationColumns, Count);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const int32 Row = Index / Columns;
		const int32 Column = Index % Columns;
		const int32 RowCount = FMath::Min(Columns, Count - Row * Columns);
		const FVector Offset(-Row * Definition->FormationSpacing, (Column - (RowCount - 1) * 0.5f) * Definition->FormationSpacing, 0.f);
		const FTransform SpawnTransform(HomeTransform.GetRotation(), HomeTransform.TransformPosition(Offset));
		ASoldierCharacter* Soldier = GetWorld()->SpawnActorDeferred<ASoldierCharacter>(Definition->SoldierClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Soldier)
		{
			Soldier->InitFromSquad(this, Index, Definition);
			FSoldierRuntime& Runtime = Soldiers.AddDefaulted_GetRef();
			Runtime.Soldier = Soldier;
			Runtime.SlotIndex = Index;
			Soldier->FinishSpawning(SpawnTransform);
		}
		if (IsActorBeingDestroyed()) { return; }
		if (!IsValid(Soldier) || Soldier->IsActorBeingDestroyed() || !Registry.IsValid() || !Registry->GetSquads().Contains(this))
		{
			ReleaseSoldiers();
			if (Registry.IsValid()) { Registry->UnregisterSquad(this); }
			UE_LOG(LogGameArmy, Error, TEXT("%s: soldier spawn failed; rolled back soldiers and squad registration."), *GetName());
			return;
		}
	}
#if !UE_BUILD_SHIPPING
	static FDelegateHandle DrawHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateStatic(&SquadSpawnDebug::Draw));
#endif
	UE_LOG(LogGameArmy, Log, TEXT("%s: registered with %d soldiers."), *GetName(), Soldiers.Num());
}

void ASquad::ReleaseSoldiers()
{
	TArray<FSoldierRuntime> Owned = MoveTemp(Soldiers);
	for (const FSoldierRuntime& Runtime : Owned)
	{
		if (Runtime.Soldier.IsValid() && !Runtime.Soldier->IsActorBeingDestroyed()) { Runtime.Soldier->Destroy(); }
	}
}

void ASquad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Registry.IsValid()) { Registry->UnregisterSquad(this); }
	Registry.Reset();
	ReleaseSoldiers();
	Super::EndPlay(EndPlayReason);
}

TArray<ASoldierCharacter*> ASquad::GetSoldiers() const
{
	TArray<ASoldierCharacter*> Result;
	for (const FSoldierRuntime& Runtime : Soldiers)
	{
		if (Runtime.Soldier.IsValid() && !Runtime.Soldier->IsActorBeingDestroyed()) { Result.Add(Runtime.Soldier.Get()); }
	}
	return Result;
}

void ASquad::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	if (Definition && Definition->SquadTypeTag.IsValid()) { TagContainer.AddTag(Definition->SquadTypeTag); }
}
