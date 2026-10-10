import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
seq = assets.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle")
src_seq = assets.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle")

length = seq.get_play_length()
for b in ["pelvis", "spine_01", "spine_02", "head", "upperarm_r", "hand_r"]:
    # Source
    s_t0 = unreal.AnimationLibrary.get_bone_pose_for_time(src_seq, b, 0.0, False)
    s_t1 = unreal.AnimationLibrary.get_bone_pose_for_time(src_seq, b, length * 0.5, False)
    s_diff = abs(s_t1.rotation.rotator().pitch - s_t0.rotation.rotator().pitch) + \
             abs(s_t1.rotation.rotator().yaw - s_t0.rotation.rotator().yaw) + \
             abs(s_t1.rotation.rotator().roll - s_t0.rotation.rotator().roll)
    
    # Target
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(seq, b, 0.0, False)
    t1 = unreal.AnimationLibrary.get_bone_pose_for_time(seq, b, length * 0.5, False)
    diff = abs(t1.rotation.rotator().pitch - t0.rotation.rotator().pitch) + \
           abs(t1.rotation.rotator().yaw - t0.rotation.rotator().yaw) + \
           abs(t1.rotation.rotator().roll - t0.rotation.rotator().roll)
    unreal.log(f"Bone {b}: Source rot delta = {s_diff:.2f} deg, Target rot delta = {diff:.2f} deg, Pose={t0.rotation.rotator()}")
