import unreal

skm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
skel = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SK_Mannequin")
manny = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny")

unreal.log(f"SKM_Warlord: {skm}")
unreal.log(f"SKM_Manny: {manny}")
