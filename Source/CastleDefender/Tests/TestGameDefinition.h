#pragma once

#include "CoreMinimal.h"
#include "Core/GameDefinition.h"
#include "TestGameDefinition.generated.h"

/** Test helper: concrete definition for GameDefinition specs. Not for content. */
UCLASS(NotBlueprintable, HideDropdown)
class UTestGameDefinition : public UGameDefinition
{
	GENERATED_BODY()
};
