"""Assigns SM_MagicSword to BP_Hero_Warlord and configures grip transform."""
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

BP_PATH = "/Game/CastleDefender/Hero/BP_Hero_Warlord"
SM_PATH = "/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword"

hero_bp = assets.load_asset(BP_PATH)
sm = assets.load_asset(SM_PATH)

if not hero_bp or not sm:
    unreal.log_error(f"Failed to load assets: BP={hero_bp}, SM={sm}")
else:
    hero_cdo = unreal.get_default_object(hero_bp.generated_class())
    weapon = hero_cdo.get_editor_property("weapon_mesh")
    
    weapon.set_editor_property("static_mesh", sm)
    # Scale: sword is 266.8 cm, scaled by 0.5 becomes 133.4 cm (natural two-handed bastard sword)
    weapon.set_editor_property("relative_scale3d", unreal.Vector(0.5, 0.5, 0.5))
    # Grip alignment: palm center offset from wrist
    weapon.set_editor_property("relative_location", unreal.Vector(4.0, 2.0, 0.0))
    # Rotation: aligns +Y blade along two-handed forward-up stance
    weapon.set_editor_property("relative_rotation", unreal.Rotator(160.0, 90.0, 80.0))
    
    unreal.BlueprintEditorLibrary.compile_blueprint(hero_bp)
    assets.save_loaded_asset(hero_bp, only_if_is_dirty=False)
    unreal.log("Configured SM_MagicSword on BP_Hero_Warlord.WeaponMesh successfully.")

    # Verification: check CDO values
    cdo_recheck = unreal.get_default_object(hero_bp.generated_class())
    wm_recheck = cdo_recheck.get_editor_property("weapon_mesh")
    unreal.log(f"Recheck static_mesh: {wm_recheck.get_editor_property('static_mesh')}")
    unreal.log(f"Recheck scale: {wm_recheck.get_editor_property('relative_scale3d')}")
    unreal.log(f"Recheck loc: {wm_recheck.get_editor_property('relative_location')}")
    unreal.log(f"Recheck rot: {wm_recheck.get_editor_property('relative_rotation')}")
