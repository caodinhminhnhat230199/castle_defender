import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))

if pawn:
    mesh = pawn.get_editor_property("mesh")
    pelvis_loc = mesh.get_socket_location("pelvis")
    hand_r_loc = mesh.get_socket_location("hand_r")
    unreal.log(f"Pelvis loc: {pelvis_loc}")
    unreal.log(f"Hand_r loc: {hand_r_loc}")
    unreal.EditorLevelLibrary.destroy_actor(pawn)
