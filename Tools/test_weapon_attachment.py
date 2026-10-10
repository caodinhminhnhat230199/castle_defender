import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    wm = pawn.get_editor_property("weapon_mesh")
    mesh = pawn.get_editor_property("mesh")
    unreal.log(f"Spawned pawn WM: {wm}")
    unreal.log(f"WM attach parent: {wm.get_attach_parent()}")
    unreal.log(f"WM attach socket: {wm.get_attach_socket_name()}")
    unreal.log(f"WM relative transform: loc={wm.get_editor_property('relative_location')}, rot={wm.get_editor_property('relative_rotation')}, scale={wm.get_editor_property('relative_scale3d')}")
    unreal.log(f"Mesh has hand_r bone? does_socket_exist: {mesh.does_socket_exist('hand_r')}")
    unreal.log(f"Mesh hand_r bone/socket loc: {mesh.get_socket_location('hand_r')}")

    unreal.EditorLevelLibrary.destroy_actor(pawn)
