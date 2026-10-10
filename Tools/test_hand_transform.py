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
    
    t_hand = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_ACTOR)
    unreal.log(f"hand_r RTS_ACTOR: loc={t_hand.translation}, rot={t_hand.rotation.rotator()}")
    
    # In UE mannequin, hand_r orientation:
    # Usually: to align a weapon extending along +Y with the hand grasp (pointing forward/out of palm or thumb/index grasp):
    # Sword blade is +Y.
    # If sword grip is held in hand:
    # Let's inspect test rotations:
    for rot in [unreal.Rotator(0, 0, 0), unreal.Rotator(0, 90, 0), unreal.Rotator(90, 0, 0), unreal.Rotator(0, 0, 90), unreal.Rotator(-90, 0, 0), unreal.Rotator(0, -90, 0), unreal.Rotator(-90, -90, 0)]:
        wm.set_editor_property("relative_rotation", rot)
        t_wm = wm.get_socket_transform("", unreal.RelativeTransformSpace.RTS_ACTOR)
        unreal.log(f"Rot {rot} -> WM RTS_ACTOR rot={t_wm.rotation.rotator()}")

    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
