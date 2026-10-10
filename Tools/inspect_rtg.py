import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg_path = "/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin"
rtg = assets.load_asset(rtg_path)
if rtg:
    unreal.log(f"RTG type: {type(rtg)}")
    for p in ["source_ik_rig_asset", "target_ik_rig_asset", "source_preview_mesh", "target_preview_mesh"]:
        try:
            val = rtg.get_editor_property(p)
            unreal.log(f"{p}: {val.get_name() if val else 'None'}")
        except Exception as e:
            unreal.log(f"{p} error: {e}")
