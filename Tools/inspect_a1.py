import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
new_seq = assets.load_asset("/Game/CastleDefender/Hero/Retargeted/UAL2_StandardSword_Regular_A1")
if new_seq:
    length = new_seq.get_editor_property("sequence_length")
    num_keys = new_seq.get_editor_property("number_of_sampled_keys")
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(new_seq, "hand_r", 0.0, False)
    t_mid = unreal.AnimationLibrary.get_bone_pose_for_time(new_seq, "hand_r", length * 0.5, False)
    unreal.log(f"NEW RETARGETED UAL2_StandardSword_Regular_A1:")
    unreal.log(f"  keys={num_keys}, len={length}")
    unreal.log(f"  hand_r 0.0s: Pos={t0.translation}, Rot={t0.rotation.rotator()}")
    unreal.log(f"  hand_r {length*0.5:.2f}s: Pos={t_mid.translation}, Rot={t_mid.rotation.rotator()}")
    
    rot_diff = abs(t_mid.rotation.rotator().pitch - t0.rotation.rotator().pitch) + \
               abs(t_mid.rotation.rotator().yaw - t0.rotation.rotator().yaw) + \
               abs(t_mid.rotation.rotator().roll - t0.rotation.rotator().roll)
    unreal.log(f"  TOTAL ROTATION DELTA: {rot_diff:.2f} degrees!")
