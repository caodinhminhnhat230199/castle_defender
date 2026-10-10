import unreal
import math

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")

pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    mesh = pawn.get_editor_property("mesh")
    t_hand_world = mesh.get_socket_transform("hand_r", unreal.RelativeTransformSpace.RTS_WORLD)
    
    loc_r = mesh.get_socket_location("hand_r")
    loc_l = mesh.get_socket_location("hand_l")
    
    # Desired blade direction in world space (from left hand to right hand)
    blade_dir_world = loc_r - loc_l
    blade_dir_world = blade_dir_world.normal()
    
    # Transform blade_dir into hand_r local space
    # hand_world_rot:
    hand_rot = t_hand_world.rotation.rotator()
    unreal.log(f"hand_r world rot: {hand_rot}")
    
    # In hand_r local space:
    # A vector along +Y in sword space should align with hand's grip direction.
    # What is the inverse transform of blade_dir_world?
    # Unreal transform vector:
    blade_dir_local = t_hand_world.inverse_transform_vector_no_scale(blade_dir_world)
    unreal.log(f"blade_dir in hand_r local space: {blade_dir_local}")
    
    # Find rotation from (0, 1, 0) (sword +Y) to blade_dir_local:
    # Let's see:
    q = unreal.Rotator(0,0,0).quaternion()
    # Or let's test Rotator from axes
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
