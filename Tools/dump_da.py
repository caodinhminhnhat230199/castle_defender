import unreal

da = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")
if da:
    light = da.get_editor_property("light_chain")
    for i, a in enumerate(light):
        m = a.get_editor_property("montage")
        unreal.log(f"Light[{i}] montage: {m} (len={m.get_play_length() if m else 0})")
        if m:
            for n in m.get_editor_property("notifies"):
                unreal.log(f"   notify: {n.get_editor_property('notify_state_class')} time={n.get_editor_property('time')}")
    heavy = da.get_editor_property("heavy")
    hm = heavy.get_editor_property("montage")
    unreal.log(f"Heavy montage: {hm} (len={hm.get_play_length() if hm else 0})")
    dodge = da.get_editor_property("dodge")
    for f in ["forward_montage", "backward_montage", "left_montage", "right_montage"]:
        dm = dodge.get_editor_property(f)
        unreal.log(f"Dodge {f}: {dm} (len={dm.get_play_length() if dm else 0})")
