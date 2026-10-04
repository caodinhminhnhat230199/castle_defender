using UnrealBuildTool;

public class CastleDefenderEditorTarget : TargetRules
{
	public CastleDefenderEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("CastleDefender");
	}
}
