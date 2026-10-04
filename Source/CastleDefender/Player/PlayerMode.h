#pragma once

#include "CoreMinimal.h"
#include "PlayerMode.generated.h"

/** Player modes owned by AHeroPlayerController (D-19). Combat is the permanent base. */
UENUM(BlueprintType)
enum class EPlayerMode : uint8
{
	Combat,
	Wheel,
	Build,
	Focus,
	Spirit,
	Modal
};

/** Push/pop stack of modes, each entry tagged with the reason that pushed it. */
struct FPlayerModeStack
{
	void Push(EPlayerMode Mode, FName Reason)
	{
		Entries.Add({ Mode, Reason });
	}

	/** Removes the most recent entry pushed with Reason, even if it is not on top. False if none. */
	bool Pop(FName Reason)
	{
		for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
		{
			if (Entries[Index].Reason == Reason)
			{
				Entries.RemoveAt(Index);
				return true;
			}
		}
		return false;
	}

	EPlayerMode Top() const
	{
		return Entries.Num() > 0 ? Entries.Last().Mode : EPlayerMode::Combat;
	}

	int32 Num() const { return Entries.Num(); }

private:
	struct FEntry
	{
		EPlayerMode Mode;
		FName Reason;
	};
	TArray<FEntry> Entries;
};
