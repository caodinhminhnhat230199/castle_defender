import unreal

m = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/AM_Warlord_Light_01")
if m:
    # check slot tracks
    slot_tracks = m.get_editor_property("slot_anim_tracks")
    if len(slot_tracks) > 0:
        track = slot_tracks[0]
        segments = track.get_editor_property("anim_track").get_editor_property("anim_segments")
        if len(segments) > 0:
            seq = segments[0].get_editor_property("anim_reference")
            unreal.log(f"Montage sequence: {seq}")
        else:
            unreal.log_error("No anim segments in montage!")
    else:
        unreal.log_error("No slot tracks in montage!")
else:
    unreal.log_error("Could not load montage")
