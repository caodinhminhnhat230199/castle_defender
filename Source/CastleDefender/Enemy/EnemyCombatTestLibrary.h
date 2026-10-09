#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnemyCombatTestLibrary.generated.h"

/**
 * Blueprint function library providing authoritative execution of the 5 Enemy Combat functional test scenarios (T-ENM-12).
 */
UCLASS()
class CASTLEDEFENDER_API UEnemyCombatTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Runs a named enemy combat functional test scenario.
	 *
	 * @param WorldContextObject Actor or object in the test world.
	 * @param ScenarioName Scenario ID (e.g. "FT_Enemy_AggroChase", "FT_Enemy_TelegraphGap", "FT_Enemy_StaggerCancel", "FT_Enemy_DeathReportOnce", "FT_Enemy_ParryStaggers")
	 * @param OutMessage Resulting message detailing verification and acceptance criteria.
	 * @return True if the scenario passed all acceptance criteria, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunEnemyCombatScenario(UObject* WorldContextObject, const FString& ScenarioName, FString& OutMessage);
};
