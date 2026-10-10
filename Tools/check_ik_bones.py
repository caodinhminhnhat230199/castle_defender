import unreal

mesh = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
comp = unreal.new_object(unreal.SkeletalMeshComponent)
comp.set_skeletal_mesh_asset(mesh)
num_bones = comp.get_num_bones()
bone_names = [str(comp.get_bone_name(i)) for i in range(num_bones)]

unreal.log(f"SKM_Warlord has {num_bones} bones.")
for ik in ["ik_foot_root", "ik_foot_r", "ik_foot_l", "ik_hand_root", "ik_hand_r", "ik_hand_l"]:
    unreal.log(f"Bone {ik} in SKM_Warlord: {ik in bone_names}")
