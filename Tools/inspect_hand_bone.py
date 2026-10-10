import unreal

skm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
comp = unreal.new_object(unreal.SkeletalMeshComponent)
comp.set_skeletal_mesh_asset(skm)

rot = comp.get_bone_quaternion("hand_r", unreal.BoneSpaces.COMPONENT_SPACE).rotator()
pos = comp.get_bone_location_by_name("hand_r", unreal.BoneSpaces.COMPONENT_SPACE)
unreal.log(f"SKM_Warlord hand_r ComponentSpace: pos={pos}, rot=({rot.pitch:.1f}, {rot.yaw:.1f}, {rot.roll:.1f})")
