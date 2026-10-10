import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    wm = pawn.get_editor_property("weapon_mesh")
    sm = wm.get_editor_property("static_mesh")
    unreal.log(f"Spawned pawn WeaponMesh static_mesh: {sm}")
    assert sm is not None, "WeaponMesh must have StaticMesh"
    
    # Check socket existence on WeaponMesh
    assert wm.does_socket_exist("Trace_Start"), "Trace_Start socket missing on WeaponMesh"
    assert wm.does_socket_exist("Trace_End"), "Trace_End socket missing on WeaponMesh"
    
    start_pos = wm.get_socket_location("Trace_Start")
    end_pos = wm.get_socket_location("Trace_End")
    dist = (end_pos - start_pos).length()
    unreal.log(f"WeaponMesh Trace_Start={start_pos}, Trace_End={end_pos}, blade length={dist:.1f} cm")
    assert 90.0 < dist < 110.0, f"Blade trace distance should be ~97.5 cm (195 cm * 0.5 scale), got {dist}"

    # Check MeleeTraceComponent
    trace_comp = pawn.get_melee_trace_component()
    unreal.log(f"MeleeTraceComponent: {trace_comp}")
    
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
    unreal.log("Verification test passed successfully!")
