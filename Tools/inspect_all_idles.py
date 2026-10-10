import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

anims = {
    'Source_Sword_Idle': '/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle',
    'Source_Idle_Loop': '/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Idle_Loop',
    'Retargeted_Sword_Idle': '/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle',
    'MM_Idle': '/Game/CastleDefender/Placeholder/Mannequins/Anims/Unarmed/MM_Idle',
}

options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)

for name, path in anims.items():
    anim = assets.load_asset(path)
    if not anim:
        unreal.log(f"Missing {name}")
        continue
    pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(anim, 0.0, options)
    unreal.log(f"=== {name} ===")
    for b in ['pelvis', 'spine_01', 'spine_03', 'upperarm_l', 'upperarm_r', 'thigh_l', 'thigh_r', 'calf_l', 'calf_r', 'foot_l', 'foot_r']:
        t = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD)
        rot = t.rotation.rotator()
        unreal.log(f"   {b:12s}: Z={t.translation.z:6.1f} | rot=({rot.pitch:6.1f}, {rot.yaw:6.1f}, {rot.roll:6.1f})")
