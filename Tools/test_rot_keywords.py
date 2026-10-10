import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

# Test with explicit keyword arguments
r = unreal.Rotator(pitch=160.0, yaw=90.0, roll=80.0)
unreal.log(f"Rotator with keywords: pitch={r.pitch}, yaw={r.yaw}, roll={r.roll}")

hero_cdo = unreal.get_default_object(bp.generated_class())
weapon = hero_cdo.get_editor_property("weapon_mesh")
weapon.set_editor_property("relative_rotation", r)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_loaded_asset(bp)

# Check in level
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))
if pawn:
    wm = pawn.get_editor_property("weapon_mesh")
    p_start = wm.get_socket_location("Trace_Start")
    p_end = wm.get_socket_location("Trace_End")
    unreal.log(f"With keyword rot: Trace_Start={p_start}, Trace_End={p_end}")
    blade_dir = (p_end - p_start).normal()
    unreal.log(f"Blade direction in world: {blade_dir}")
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor_sub.destroy_actor(pawn)
