#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HeroCombatTestLibrary.generated.h"

/**
 * Blueprint function library providing authoritative execution of the 16 Hero Combat functional test scenarios (T-CMB-15).
 */
UCLASS()
class CASTLEDEFENDER_API UHeroCombatTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Runs a named hero combat functional test scenario.
	 *
	 * @param WorldContextObject Actor or object in the test world.
	 * @param ScenarioName Scenario ID (e.g. "FT_LightChain", "FT_OneHitPerSwing", etc.)
	 * @param OutMessage Resulting message detailing verification and acceptance criteria.
	 * @return True if the scenario passed all acceptance criteria, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunHeroCombatScenario(UObject* WorldContextObject, const FString& ScenarioName, FString& OutMessage);
};
