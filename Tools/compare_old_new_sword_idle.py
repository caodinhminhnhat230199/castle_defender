import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

old_anim = assets.load_asset('/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle')
new_anim = assets.load_asset('/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle1')

options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)

if old_anim and new_anim:
    p_old = unreal.AnimPoseExtensions.get_anim_pose_at_time(old_anim, 0.0, options)
    p_new = unreal.AnimPoseExtensions.get_anim_pose_at_time(new_anim, 0.0, options)
    
    unreal.log("=== COMPARE OLD (13 ops) VS NEW (clean ops) ===")
    for b in ['pelvis', 'spine_01', 'spine_02', 'spine_03', 'thigh_l', 'thigh_r', 'calf_l', 'calf_r', 'upperarm_l', 'upperarm_r']:
        t_o = unreal.AnimPoseExtensions.get_bone_pose(p_old, b, unreal.AnimPoseSpaces.WORLD)
        t_n = unreal.AnimPoseExtensions.get_bone_pose(p_new, b, unreal.AnimPoseSpaces.WORLD)
        rot_o = t_o.rotation.rotator()
        rot_n = t_n.rotation.rotator()
        unreal.log(f"[{b}]")
        unreal.log(f"   OLD: Z={t_o.translation.z:6.1f} | rot=({rot_o.pitch:6.1f}, {rot_o.yaw:6.1f}, {rot_o.roll:6.1f})")
        unreal.log(f"   NEW: Z={t_n.translation.z:6.1f} | rot=({rot_n.pitch:6.1f}, {rot_n.yaw:6.1f}, {rot_n.roll:6.1f})")
else:
    unreal.log(f"old={old_anim}, new={new_anim}")
