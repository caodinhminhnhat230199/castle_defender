import unreal

bs = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BS_Warlord_Strafe")
if bs:
    samples = bs.get_editor_property("sample_data")
    for i, s in enumerate(samples):
        anim = s.get_editor_property("animation")
        val = s.get_editor_property("sample_value")
        unreal.log(f"Sample {i} at {val}: {anim}")
else:
    unreal.log_error("No BS_Warlord_Strafe")
