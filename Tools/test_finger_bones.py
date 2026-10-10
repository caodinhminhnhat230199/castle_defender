import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    mesh = pawn.get_editor_property("mesh")
    # Let's inspect index finger and thumb of hand_r and hand_l
    for b in ["hand_r", "index_01_r", "thumb_01_r", "hand_l", "index_01_l", "thumb_01_l"]:
        if mesh.does_socket_exist(b):
            loc = mesh.get_socket_location(b)
            unreal.log(f"Bone {b}: loc={loc}")
            
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
