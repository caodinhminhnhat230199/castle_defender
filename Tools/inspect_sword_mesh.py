import unreal

sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")
if sm:
    bounds = sm.get_bounds()
    unreal.log(f"SM_MagicSword bounds: origin={bounds.origin}, box_extent={bounds.box_extent}")
    unreal.log(f"Dimensions: X={bounds.box_extent.x * 2:.1f}, Y={bounds.box_extent.y * 2:.1f}, Z={bounds.box_extent.z * 2:.1f}")
    unreal.log(f"Materials: {sm.get_editor_property('static_materials')}")
