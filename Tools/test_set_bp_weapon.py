import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

if bp and sm:
    cdo = unreal.get_default_object(bp.generated_class())
    wm = cdo.get_editor_property("weapon_mesh")
    unreal.log(f"Current wm: {wm}")
    wm.set_editor_property("static_mesh", sm)
    wm.set_editor_property("relative_scale3d", unreal.Vector(0.5, 0.5, 0.5))
    unreal.log(f"Updated CDO wm static mesh: {wm.get_editor_property('static_mesh')}")
    unreal.log(f"Updated CDO wm scale: {wm.get_editor_property('relative_scale3d')}")

    # Also check if BP has component templates or SCS nodes
    scs = bp.get_editor_property("simple_construction_script")
    unreal.log(f"SCS: {scs}")
    if scs:
        for node in scs.get_all_nodes():
            unreal.log(f"SCS node: {node.get_name()}, comp: {node.get_editor_property('component_class')}")

    # Check inherited component templates
    # In UE, components added in C++ constructor are in bp.component_templates or inherited
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
