import unreal

mesh = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
m1 = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/AM_Warlord_Light_01")
abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")

sk_mesh = mesh.get_editor_property("skeleton")
sk_m1 = m1.get_editor_property("skeleton")
sk_abp = abp.get_editor_property("target_skeleton")

unreal.log(f"SKM_Warlord Skeleton: {sk_mesh.get_path_name()} ({sk_mesh})")
unreal.log(f"AM_Warlord_Light_01 Skeleton: {sk_m1.get_path_name()} ({sk_m1})")
unreal.log(f"ABP_Warlord Skeleton: {sk_abp.get_path_name()} ({sk_abp})")
unreal.log(f"Skeletons Match: {sk_mesh == sk_m1 == sk_abp}")
