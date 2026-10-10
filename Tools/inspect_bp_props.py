import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
cdo = unreal.get_default_object(bp.generated_class())
unreal.log(f"CDO: {cdo}")
wm = cdo.get_editor_property("weapon_mesh")
unreal.log(f"CDO wm: {wm}")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

# Let's inspect blueprint properties
unreal.log(f"bp properties: {[p for p in dir(bp) if 'comp' in p.lower() or 'node' in p.lower() or 'scs' in p.lower()]}")

# Also check component_templates on Blueprint
if hasattr(bp, 'component_templates'):
    unreal.log(f"component_templates: {bp.component_templates}")
