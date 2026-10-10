import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for anim_name in ["AL_StandardRig_Idle_Loop", "AL_StandardRig_Sword_Idle"]:
    anim = assets.load_asset(f"/Game/CastleDefender/Assets/UAL_Standard/{anim_name}")
    if anim:
        options = unreal.AnimPoseEvaluationOptions()
        options.set_editor_property('extract_root_motion', False)
        pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(anim, 0.0, options)
        pelvis = unreal.AnimPoseExtensions.get_bone_pose(pose, 'pelvis', unreal.AnimPoseSpaces.WORLD)
        unreal.log(f"{anim_name}: len={anim.get_play_length():.2f}s pelvis_z={pelvis.translation.z:.1f} rot=({pelvis.rotation.rotator().pitch:.1f}, {pelvis.rotation.rotator().yaw:.1f}, {pelvis.rotation.rotator().roll:.1f})")
