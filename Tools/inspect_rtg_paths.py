import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for rtg_path in ["/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin", "/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin"]:
    rtg = assets.load_asset(rtg_path)
    if rtg:
        src = rtg.get_editor_property("source_ik_rig_asset")
        tgt = rtg.get_editor_property("target_ik_rig_asset")
        unreal.log(f"{rtg.get_name()}:")
        unreal.log(f"  Source: {src.get_path_name() if src else 'None'}")
        unreal.log(f"  Target: {tgt.get_path_name() if tgt else 'None'}")
