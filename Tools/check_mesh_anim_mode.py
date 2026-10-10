import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
cdo = unreal.get_default_object(bp.generated_class())
mesh = cdo.get_editor_property("mesh")

unreal.log(f"Animation Mode: {mesh.get_editor_property('animation_mode')}")
unreal.log(f"Anim Class: {mesh.get_editor_property('anim_class')}")
unreal.log(f"Visibility: {mesh.get_editor_property('visible')}")
unreal.log(f"Hidden In Game: {mesh.get_editor_property('hidden_in_game')}")
