using UnrealBuildTool;

public class CastleDefender : ModuleRules
{
	public CastleDefender(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Headers sit next to sources in domain folders (foundation technical-plan §4),
		// so includes are written as "Combat/HealthComponent.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags", "AIModule", "DeveloperSettings" });

		PrivateDependencyModuleNames.AddRange(new string[] { "NavigationSystem", "UMG", "Slate", "SlateCore" });
	}
}
