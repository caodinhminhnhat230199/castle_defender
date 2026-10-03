#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FGameTagsSpec, "CastleDefender.Core.GameTags", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FGameTagsSpec)

void FGameTagsSpec::Define()
{
	It("registers every main plan section 8 root and leaf with the tag manager", [this]()
	{
		const TCHAR* Names[] = {
			TEXT("State.Combat.Staggered"), TEXT("State.Combat.ArmorBroken"), TEXT("State.Combat.Marked"),
			TEXT("State.Hero.Attacking"), TEXT("State.Hero.Dodging"), TEXT("State.Hero.Blocking"), TEXT("State.Hero.Parrying"), TEXT("State.Hero.Dead"),
			TEXT("Unit.Enemy.Swarm"), TEXT("Unit.Enemy.Armored"), TEXT("Unit.Enemy.Siege"), TEXT("Unit.Enemy.Boss"),
			TEXT("Unit.Squad.Infantry"), TEXT("Unit.Squad.Archer"), TEXT("Unit.Squad.Spearman"),
			TEXT("Structure.Role.Core"), TEXT("Structure.Role.Blocker"), TEXT("Structure.Role.CombatTower"), TEXT("Structure.Role.Utility"),
			TEXT("Command.Guard"), TEXT("Command.Attack"), TEXT("Command.Follow"), TEXT("Command.Retreat"),
			TEXT("Zone.Bridge"), TEXT("Zone.Gate"), TEXT("Zone.HighGround"), TEXT("Zone.Chokepoint"), TEXT("Zone.ResourceCamp"), TEXT("Zone.RallyPoint"),
			TEXT("Damage.Physical"),
			TEXT("Perk.Category.Hero"), TEXT("Perk.Category.Army"), TEXT("Perk.Category.Defense"),
			TEXT("Stat"), TEXT("Feedback"), TEXT("Modifier.Encounter"), TEXT("Lane"), TEXT("Resource"), TEXT("Tutorial.Gate"),
		};

		for (const TCHAR* Name : Names)
		{
			TestTrue(Name, UGameplayTagsManager::Get().RequestGameplayTag(FName(Name), false).IsValid());
		}
	});
}

#endif
