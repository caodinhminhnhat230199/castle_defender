#include "Core/GameTuningSettings.h"

#include "Core/GameTags.h"

UGameTuningSettings::UGameTuningSettings()
{
	// World-time fallback durations; explicit hit/unit durations take precedence.
	StateDefaultDurations.Add(GameTags::State_Combat_Staggered, 1.5f);
	StateDefaultDurations.Add(GameTags::State_Combat_ArmorBroken, 6.f);
}
