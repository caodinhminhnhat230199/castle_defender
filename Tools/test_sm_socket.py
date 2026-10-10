import unreal

sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")
if sm:
    unreal.log(f"Help on StaticMeshSocket: {dir(unreal.StaticMeshSocket)}")
    # create socket
    s_start = unreal.StaticMeshSocket(sm)
    s_start.set_editor_property("socket_name", "Trace_Start")
    s_start.set_editor_property("relative_location", unreal.Vector(0.0, 35.0, 0.0))
    sm.add_socket(s_start)
    
    s_end = unreal.StaticMeshSocket(sm)
    s_end.set_editor_property("socket_name", "Trace_End")
    s_end.set_editor_property("relative_location", unreal.Vector(0.0, 230.0, 0.0))
    sm.add_socket(s_end)

    # also add weapon_base and weapon_tip in case anything uses those fallback names
    s_base = unreal.StaticMeshSocket(sm)
    s_base.set_editor_property("socket_name", "weapon_base")
    s_base.set_editor_property("relative_location", unreal.Vector(0.0, 35.0, 0.0))
    sm.add_socket(s_base)

    s_tip = unreal.StaticMeshSocket(sm)
    s_tip.set_editor_property("socket_name", "weapon_tip")
    s_tip.set_editor_property("relative_location", unreal.Vector(0.0, 230.0, 0.0))
    sm.add_socket(s_tip)

    unreal.EditorAssetLibrary.save_loaded_asset(sm)
    
    unreal.log(f"Verification - find Trace_Start: {sm.find_socket('Trace_Start')}")
    unreal.log(f"Verification - find Trace_End: {sm.find_socket('Trace_End')}")
