#include "Player/CommandComponent.h"
#include "Army/Squad.h"
#include "Core/GameTuningSettings.h"

UCommandComponent::UCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCommandComponent::RegisterSquad(ASquad* Squad)
{
	if (!IsValid(Squad) || Squad->IsActorBeingDestroyed() || !GetWorld() || Squad->GetWorld() != GetWorld()) { return false; }
	Squads.RemoveAll([](const TWeakObjectPtr<ASquad>& Item) { return !Item.IsValid() || Item->IsActorBeingDestroyed(); });
	if (Squads.Contains(Squad)) { return true; }
	if (Squads.Num() >= UGameTuningSettings::Get()->MaxActiveSquads) { return false; }
	Squads.Add(Squad);
	OnSquadsChanged.Broadcast();
	return true;
}

void UCommandComponent::UnregisterSquad(ASquad* Squad)
{
	const int32 Removed = Squads.RemoveAll([Squad](const TWeakObjectPtr<ASquad>& Item)
	{
		return Item.Get() == Squad || !Item.IsValid() || Item->IsActorBeingDestroyed();
	});
	if (Removed > 0) { OnSquadsChanged.Broadcast(); }
}

TArray<ASquad*> UCommandComponent::GetSquads() const
{
	TArray<ASquad*> Result;
	for (const TWeakObjectPtr<ASquad>& Squad : Squads)
	{
		if (Squad.IsValid() && !Squad->IsActorBeingDestroyed()) { Result.Add(Squad.Get()); }
	}
	return Result;
}
