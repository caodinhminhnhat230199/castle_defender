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
    
    loc_r = mesh.get_socket_location("hand_r")
    loc_l = mesh.get_socket_location("hand_l")
    desired_dir = (loc_r - loc_l).normal()
    unreal.log(f"Desired blade direction in world space: {desired_dir}")
    
    best_dot = -2.0
    best_rot = None
    
    # 90-degree step search
    for pitch in [0, 90, 180, 270]:
        for yaw in [0, 90, 180, 270]:
            for roll in [0, 90, 180, 270]:
                rot = unreal.Rotator(pitch, yaw, roll)
                wm.set_editor_property("relative_rotation", rot)
                p_start = wm.get_socket_location("Trace_Start")
                p_end = wm.get_socket_location("Trace_End")
                blade_axis = (p_end - p_start).normal()
                dot = desired_dir.dot(blade_axis)
                if dot > best_dot:
                    best_dot = dot
                    best_rot = rot
                    
    unreal.log(f"Best coarse rotation: {best_rot} with dot={best_dot:.4f}")
    
    # Fine search around best_rot
    fine_best_dot = best_dot
    fine_best_rot = best_rot
    for dp in range(-45, 50, 5):
        for dy in range(-45, 50, 5):
            for dr in range(-45, 50, 5):
                rot = unreal.Rotator(best_rot.pitch + dp, best_rot.yaw + dy, best_rot.roll + dr)
                wm.set_editor_property("relative_rotation", rot)
                p_start = wm.get_socket_location("Trace_Start")
                p_end = wm.get_socket_location("Trace_End")
                blade_axis = (p_end - p_start).normal()
                dot = desired_dir.dot(blade_axis)
                if dot > fine_best_dot:
                    fine_best_dot = dot
                    fine_best_rot = rot
                    
    unreal.log(f"Fine best rotation: {fine_best_rot} with dot={fine_best_dot:.4f}")
    
    # Now check hilt location relative to hand_r
    # We want the sword grip (Y ~ 0) to align with hand_r loc
    wm.set_editor_property("relative_rotation", fine_best_rot)
    p_start = wm.get_socket_location("Trace_Start")
    p_end = wm.get_socket_location("Trace_End")
    unreal.log(f"At fine_best_rot: Trace_Start={p_start}, Trace_End={p_end}")
    
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
