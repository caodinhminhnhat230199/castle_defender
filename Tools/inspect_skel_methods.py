import unreal

skm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
skel = skm.get_editor_property("skeleton")

# Check if we can inspect bone names or reference skeleton
unreal.log(f"SKM methods: {[m for m in dir(skm) if 'bone' in m.lower() or 'socket' in m.lower()]}")
unreal.log(f"Skeleton methods: {[m for m in dir(skel) if 'bone' in m.lower() or 'socket' in m.lower()]}")

# Check unreal.AnimationBlueprintLibrary or unreal.AnimationLibrary
unreal.log(f"unreal.AnimationBlueprintLibrary: {[m for m in dir(unreal.AnimationBlueprintLibrary) if 'bone' in m.lower() or 'pose' in m.lower()]}")
