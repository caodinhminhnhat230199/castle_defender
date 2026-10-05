#include "Core/GameTuningSettings.h"

#include "Core/GameTags.h"

UGameTuningSettings::UGameTuningSettings()
{
	// T-SYN-01 fallback for Staggered; T-SYN-02/03 add Armor Broken and Marked.
	StateDefaultDurations.Add(GameTags::State_Combat_Staggered, 1.5f);
}
