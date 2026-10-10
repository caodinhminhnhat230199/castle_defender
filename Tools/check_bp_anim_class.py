import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
if bp:
    cdo = unreal.get_default_object(bp.generated_class())
    mesh = cdo.get_editor_property("mesh")
    anim_class = mesh.get_editor_property("anim_class")
    if anim_class:
        unreal.log(f"Anim Class is: {anim_class.get_name()}")
    else:
        unreal.log_error("Anim Class is NONE!")
else:
    unreal.log_error("No BP_Hero_Warlord")
