#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameDefinition.generated.h"

/**
 * Base for every content definition (foundation technical-plan §9). Never written at runtime.
 * Primary Asset Type = native class name without the U prefix (e.g. HeroClassDefinition).
 * Register each type in DefaultGame.ini [/Script/Engine.AssetManagerSettings] when it lands.
 */
UCLASS(Abstract)
class CASTLEDEFENDER_API UGameDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Definition")
	FText DisplayName;
};
