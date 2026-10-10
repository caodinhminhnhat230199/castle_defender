import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    mesh = pawn.get_editor_property("mesh")
    wm = pawn.get_editor_property("weapon_mesh")
    wm.set_editor_property("static_mesh", sm)
    wm.set_editor_property("relative_scale3d", unreal.Vector(0.5, 0.5, 0.5))
    
    # Target: blade points forward (+X) and slightly up (+Z) and slightly right (+Y)
    desired_dirs = [
        ("ForwardUp", unreal.Vector(0.8, 0.2, 0.55).normal()),
        ("ForwardFlat", unreal.Vector(0.95, 0.2, 0.2).normal()),
        ("GedanLow", unreal.Vector(0.8, 0.2, -0.55).normal()),
    ]
    
    for label, target_dir in desired_dirs:
        best_dot = -2.0
        best_rot = None
        for pitch in range(-180, 180, 10):
            for yaw in range(-180, 180, 10):
                for roll in [-90, 0, 90, 180]:
                    rot = unreal.Rotator(pitch, yaw, roll)
                    wm.set_editor_property("relative_rotation", rot)
                    p_start = wm.get_socket_location("Trace_Start")
                    p_end = wm.get_socket_location("Trace_End")
                    blade_axis = (p_end - p_start).normal()
                    dot = target_dir.dot(blade_axis)
                    if dot > best_dot:
                        best_dot = dot
                        best_rot = rot
        unreal.log(f"Target '{label}': best_rot={best_rot} with dot={best_dot:.4f}")
        
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
