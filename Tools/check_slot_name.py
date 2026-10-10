import unreal

m = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/AM_Warlord_Light_01")
slot_name = m.get_editor_property("slot_anim_tracks")[0].get_editor_property("slot_name")
unreal.log(f"AM_Warlord_Light_01 Slot Name: {slot_name}")
