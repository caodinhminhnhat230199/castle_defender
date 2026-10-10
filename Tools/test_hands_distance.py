import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    mesh = pawn.get_editor_property("mesh")
    # evaluate pose
    loc_r = mesh.get_socket_location("hand_r")
    rot_r = mesh.get_socket_rotation("hand_r")
    loc_l = mesh.get_socket_location("hand_l")
    rot_l = mesh.get_socket_rotation("hand_l")
    unreal.log(f"hand_r loc={loc_r}, rot={rot_r}")
    unreal.log(f"hand_l loc={loc_l}, rot={rot_l}")
    diff = loc_r - loc_l
    unreal.log(f"Vector from hand_l to hand_r: {diff}, distance={diff.length():.1f} cm")

    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
