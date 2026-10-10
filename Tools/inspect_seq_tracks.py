import unreal

source_path = "/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Regular_A"
seq = unreal.EditorAssetLibrary.load_asset(source_path)
if seq:
    length = seq.get_editor_property("sequence_length")
    num_keys = seq.get_editor_property("number_of_sampled_keys")
    unreal.log(f"SOURCE SEQ: length={length}, keys={num_keys}")
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "hand_r", 0.0, False)
    t_mid = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "hand_r", length * 0.5, False)
    unreal.log(f"hand_r at 0.0s: Pos={t0.translation}, Rot={t0.rotation.rotator()}")
    unreal.log(f"hand_r at {length*0.5:.2f}s: Pos={t_mid.translation}, Rot={t_mid.rotation.rotator()}")
else:
    unreal.log(f"Failed to load source seq at {source_path}")
