#include "Feedback/FeedbackTags.h"

namespace FeedbackTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Hit_Light, "Feedback.Combat.Hit.Light", "Light attack landed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Hit_Light_Armored, "Feedback.Combat.Hit.Light.Armored", "Variant: light attack on an armored target.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Hit_Heavy, "Feedback.Combat.Hit.Heavy", "Heavy attack landed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Hit_Heavy_Armored, "Feedback.Combat.Hit.Heavy.Armored", "Variant: heavy attack on an armored target.");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Block, "Feedback.Combat.Block");
	UE_DEFINE_GAMEPLAY_TAG(Combat_BlockBreak, "Feedback.Combat.BlockBreak");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Parry, "Feedback.Combat.Parry");

	UE_DEFINE_GAMEPLAY_TAG(Hero_Damaged, "Feedback.Hero.Damaged");
	UE_DEFINE_GAMEPLAY_TAG(Hero_Death, "Feedback.Hero.Death");
	UE_DEFINE_GAMEPLAY_TAG(Hero_StaminaInsufficient, "Feedback.Hero.StaminaInsufficient");
	UE_DEFINE_GAMEPLAY_TAG(Hero_LowHealth, "Feedback.Hero.LowHealth");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Telegraph, "Feedback.Enemy.Telegraph");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Enemy_Telegraph_Heavy, "Feedback.Enemy.Telegraph.Heavy", "Variant: heavy attack telegraph.");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Death, "Feedback.Enemy.Death");

	UE_DEFINE_GAMEPLAY_TAG(State_Staggered_Applied, "Feedback.State.Staggered.Applied");
	UE_DEFINE_GAMEPLAY_TAG(State_Staggered_Removed, "Feedback.State.Staggered.Removed");
}
