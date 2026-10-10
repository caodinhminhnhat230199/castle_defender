import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin")
ctrl = unreal.IKRetargeterController.get_controller(rtg)
ctrl.add_default_ops()
ctrl.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
assets.save_loaded_asset(rtg, only_if_is_dirty=False)

# Retarget UAL2_StandardSword_Regular_A
seq_asset_data = assets.find_asset_data("/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Regular_A")
source_mesh = assets.load_asset("/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_Standard")
target_mesh = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

inputs = unreal.IKRetargetBatchOperationInputs()
inputs.set_editor_property("assets_to_retarget", [seq_asset_data])
inputs.set_editor_property("ik_retarget_asset", rtg)
inputs.set_editor_property("source_mesh", source_mesh)
inputs.set_editor_property("target_mesh", target_mesh)
inputs.set_editor_property("target_path", "/Game/CastleDefender/Hero/Retargeted")
inputs.set_editor_property("overwrite_existing_files", True)

res = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
unreal.log(f"Batch retarget result count: {len(res)}")
for r in res:
    unreal.log(f"  Result asset: {r.package_name}")

# Inspect newly retargeted sequence
new_seq = assets.load_asset("/Game/CastleDefender/Hero/Retargeted/UAL2_StandardSword_Regular_A")
if new_seq:
    length = new_seq.get_editor_property("sequence_length")
    num_keys = new_seq.get_editor_property("number_of_sampled_keys")
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(new_seq, "hand_r", 0.0, False)
    t_mid = unreal.AnimationLibrary.get_bone_pose_for_time(new_seq, "hand_r", length * 0.5, False)
    unreal.log(f"NEW RETARGETED UAL2_StandardSword_Regular_A:")
    unreal.log(f"  keys={num_keys}, len={length}")
    unreal.log(f"  hand_r 0.0s: Pos={t0.translation}, Rot={t0.rotation.rotator()}")
    unreal.log(f"  hand_r {length*0.5:.2f}s: Pos={t_mid.translation}, Rot={t_mid.rotation.rotator()}")
    
    rot_diff = abs(t_mid.rotation.rotator().pitch - t0.rotation.rotator().pitch) + \
               abs(t_mid.rotation.rotator().yaw - t0.rotation.rotator().yaw) + \
               abs(t_mid.rotation.rotator().roll - t0.rotation.rotator().roll)
    unreal.log(f"  TOTAL ROTATION DELTA: {rot_diff:.2f} degrees!")
