import unreal

src_anim = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle')
tgt_anim = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle')

options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)

src_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(src_anim, 0.0, options)
tgt_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(tgt_anim, 0.0, options)

bones = ['pelvis', 'spine_01', 'spine_02', 'spine_03', 'clavicle_l', 'clavicle_r', 'upperarm_l', 'upperarm_r', 'thigh_l', 'thigh_r', 'calf_l', 'calf_r', 'foot_l', 'foot_r']

unreal.log("=== BONE POSES AT T=0 (WORLD / COMPONENT SPACE) ===")
for b in bones:
    try:
        src_t = unreal.AnimPoseExtensions.get_bone_pose(src_pose, b, unreal.AnimPoseSpaces.WORLD)
        tgt_t = unreal.AnimPoseExtensions.get_bone_pose(tgt_pose, b, unreal.AnimPoseSpaces.WORLD)
        unreal.log(f"Bone {b}:")
        unreal.log(f"   SRC: pos=({src_t.translation.x:.1f}, {src_t.translation.y:.1f}, {src_t.translation.z:.1f}) rot=({src_t.rotation.rotator().pitch:.1f}, {src_t.rotation.rotator().yaw:.1f}, {src_t.rotation.rotator().roll:.1f})")
        unreal.log(f"   TGT: pos=({tgt_t.translation.x:.1f}, {tgt_t.translation.y:.1f}, {tgt_t.translation.z:.1f}) rot=({tgt_t.rotation.rotator().pitch:.1f}, {tgt_t.rotation.rotator().yaw:.1f}, {tgt_t.rotation.rotator().roll:.1f})")
    except Exception as e:
        unreal.log(f"Bone {b} error: {e}")
