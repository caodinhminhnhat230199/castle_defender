import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
if bp:
    cdo = unreal.get_default_object(bp.generated_class())
    unreal.log(f"HeroClassDefinition: {cdo.get_editor_property('hero_class_definition')}")
    unreal.log(f"MoveAction: {cdo.get_editor_property('move_action')}")
    unreal.log(f"LookAction: {cdo.get_editor_property('look_action')}")
    unreal.log(f"LightAttackAction: {cdo.get_editor_property('light_attack_action')}")
    unreal.log(f"HeavyAttackAction: {cdo.get_editor_property('heavy_attack_action')}")
    unreal.log(f"DodgeAction: {cdo.get_editor_property('dodge_action')}")
    unreal.log(f"BlockAction: {cdo.get_editor_property('block_action')}")
    mesh = cdo.get_editor_property("mesh")
    unreal.log(f"Mesh: {mesh.get_editor_property('skeletal_mesh_asset') if hasattr(mesh, 'get_editor_property') else 'None'}")
    unreal.log(f"AnimClass: {mesh.get_editor_property('anim_class')}")
