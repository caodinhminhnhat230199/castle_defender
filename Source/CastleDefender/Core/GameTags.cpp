#include "Core/GameTags.h"

namespace GameTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Staggered, "State.Combat.Staggered", "Poise broken; target is open to follow-up hits.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_ArmorBroken, "State.Combat.ArmorBroken", "Armor reduced for a duration.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Marked, "State.Combat.Marked", "Target is marked for focus fire.");

	UE_DEFINE_GAMEPLAY_TAG(State_Hero_Attacking, "State.Hero.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Hero_Dodging, "State.Hero.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(State_Hero_Blocking, "State.Hero.Blocking");
	UE_DEFINE_GAMEPLAY_TAG(State_Hero_Parrying, "State.Hero.Parrying");
	UE_DEFINE_GAMEPLAY_TAG(State_Hero_Dead, "State.Hero.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Unit_Enemy_Swarm, "Unit.Enemy.Swarm");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Enemy_Armored, "Unit.Enemy.Armored");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Enemy_Siege, "Unit.Enemy.Siege");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Enemy_Boss, "Unit.Enemy.Boss");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Squad_Infantry, "Unit.Squad.Infantry");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Squad_Archer, "Unit.Squad.Archer");
	UE_DEFINE_GAMEPLAY_TAG(Unit_Squad_Spearman, "Unit.Squad.Spearman");

	UE_DEFINE_GAMEPLAY_TAG(Structure_Role_Core, "Structure.Role.Core");
	UE_DEFINE_GAMEPLAY_TAG(Structure_Role_Blocker, "Structure.Role.Blocker");
	UE_DEFINE_GAMEPLAY_TAG(Structure_Role_CombatTower, "Structure.Role.CombatTower");
	UE_DEFINE_GAMEPLAY_TAG(Structure_Role_Utility, "Structure.Role.Utility");

	UE_DEFINE_GAMEPLAY_TAG(Command_Guard, "Command.Guard");
	UE_DEFINE_GAMEPLAY_TAG(Command_Attack, "Command.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Command_Follow, "Command.Follow");
	UE_DEFINE_GAMEPLAY_TAG(Command_Retreat, "Command.Retreat");

	UE_DEFINE_GAMEPLAY_TAG(Zone_Bridge, "Zone.Bridge");
	UE_DEFINE_GAMEPLAY_TAG(Zone_Gate, "Zone.Gate");
	UE_DEFINE_GAMEPLAY_TAG(Zone_HighGround, "Zone.HighGround");
	UE_DEFINE_GAMEPLAY_TAG(Zone_Chokepoint, "Zone.Chokepoint");
	UE_DEFINE_GAMEPLAY_TAG(Zone_ResourceCamp, "Zone.ResourceCamp");
	UE_DEFINE_GAMEPLAY_TAG(Zone_RallyPoint, "Zone.RallyPoint");

	UE_DEFINE_GAMEPLAY_TAG(Damage_Physical, "Damage.Physical");

	UE_DEFINE_GAMEPLAY_TAG(Perk_Category_Hero, "Perk.Category.Hero");
	UE_DEFINE_GAMEPLAY_TAG(Perk_Category_Army, "Perk.Category.Army");
	UE_DEFINE_GAMEPLAY_TAG(Perk_Category_Defense, "Perk.Category.Defense");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat, "Stat", "Root: Stat.<Domain>.<Name>, perk and zone modifier targets.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Feedback, "Feedback", "Root: Feedback.<Event>, keys into DT_Feedback.");
	UE_DEFINE_GAMEPLAY_TAG(Feedback_Hero_StaminaInsufficient, "Feedback.Hero.StaminaInsufficient");
	UE_DEFINE_GAMEPLAY_TAG(Feedback_Hero_Death, "Feedback.Hero.Death");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modifier_Encounter, "Modifier.Encounter", "Root: Director encounter modifiers.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Lane, "Lane", "Root: Lane.<Name>, lane identity for routes, spawners, forecast.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Resource, "Resource", "Root: Resource.<Name>, VS economy resources.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tutorial_Gate, "Tutorial.Gate", "Root: Tutorial.Gate.<Name>, VS onboarding gates.");
}
