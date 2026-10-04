using UnrealBuildTool;

public class CastleDefenderTarget : TargetRules
{
	public CastleDefenderTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("CastleDefender");
	}
}
