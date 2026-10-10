import unreal

bs = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BS_Warlord_Strafe")
idle_seq = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle")

if bs and idle_seq:
    samples = bs.get_editor_property("sample_data")
    modified = False
    for i, s in enumerate(samples):
        val = s.get_editor_property("sample_value")
        if abs(val.x) < 0.01 and abs(val.y) < 0.01:
            s.set_editor_property("animation", idle_seq)
            modified = True
            unreal.log("Updated sample 0")
    if modified:
        bs.set_editor_property("sample_data", samples)
        unreal.EditorAssetLibrary.save_loaded_asset(bs, only_if_is_dirty=False)
        unreal.log("Saved BS_Warlord_Strafe")
