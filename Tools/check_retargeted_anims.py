import unreal

folder = "/Game/CastleDefender/Hero/Retargeted"
assets = unreal.EditorAssetLibrary.list_assets(folder)
for a in assets:
    obj = unreal.EditorAssetLibrary.load_asset(a)
    if isinstance(obj, unreal.AnimSequence):
        unreal.log(f"Anim: {obj.get_name()}, Length: {obj.get_play_length()}, Skeleton: {obj.get_editor_property('skeleton').get_name()}")
