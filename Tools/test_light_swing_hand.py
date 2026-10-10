import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    da = pawn.get_hero_class_definition()
    light0 = da.get_editor_property("light_chain")[0].get_editor_property("montage")
    mesh = pawn.get_editor_property("mesh")
    anim_inst = mesh.get_anim_instance()
    
    # Play montage and step pose
    anim_inst.montage_play(light0)
    # Montage hit window for light0:
    # Let's inspect hand_r bone location and rotation at 0.15s, 0.20s, 0.25s
    for t in [0.0, 0.15, 0.20, 0.25]:
        anim_inst.montage_set_position(light0, t)
        mesh.refresh_bone_transforms()
        loc_r = mesh.get_socket_location("hand_r")
        rot_r = mesh.get_socket_rotation("hand_r")
        unreal.log(f"Light0 at {t:.2f}s: hand_r loc={loc_r}, rot={rot_r}")
        
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
