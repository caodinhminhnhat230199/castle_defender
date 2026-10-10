import unreal

src_mesh = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Assets/UAL_Standard/AL_Standard')
tgt_mesh = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord')

unreal.log(f"src_mesh: {src_mesh}")
unreal.log(f"tgt_mesh: {tgt_mesh}")

unreal.log(f"src skeleton: {src_mesh.get_editor_property('skeleton')}")
unreal.log(f"tgt skeleton: {tgt_mesh.get_editor_property('skeleton')}")

# Check reference pose of both
src_skel = src_mesh.get_editor_property('skeleton')
tgt_skel = tgt_mesh.get_editor_property('skeleton')

for b in ['pelvis', 'spine_01', 'spine_02', 'spine_03', 'thigh_l', 'thigh_r', 'calf_l', 'calf_r', 'foot_l', 'foot_r', 'upperarm_l', 'upperarm_r']:
    try:
        # Get ref pose bone transform from skeleton or mesh
        ref_t_src = unreal.AnimationLibrary.get_bone_pose_for_frame(src_mesh, b, 0, False)
    except:
        pass

# Check dir(ctrl)
rtg = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin')
ctrl = unreal.IKRetargeterController.get_controller(rtg)
unreal.log(f"IKRetargeterController methods: {[m for m in dir(ctrl) if 'pose' in m.lower()]}")
