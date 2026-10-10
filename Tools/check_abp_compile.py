import unreal

abp_path = "/Game/CastleDefender/Hero/ABP_Warlord"
abp = unreal.EditorAssetLibrary.load_asset(abp_path)
if abp:
    unreal.BlueprintEditorLibrary.compile_blueprint(abp)
    # Get the generated class
    cls = abp.generated_class()
    if not cls:
        unreal.log_error("Generated class is None. Blueprint failed to compile.")
    else:
        unreal.log(f"Compiled successfully: {cls}")
else:
    unreal.log_error(f"Failed to load {abp_path}")
