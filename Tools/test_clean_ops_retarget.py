import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for rtg_path in ["/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin", "/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin"]:
    rtg = assets.load_asset(rtg_path)
    ctrl = unreal.IKRetargeterController.get_controller(rtg)
    
    unreal.log(f"Cleaning {rtg.get_name()}, before ops count: {ctrl.get_num_retarget_ops()}")
    # Remove all ops
    ctrl.remove_all_ops()
    ctrl.add_default_ops()
    ctrl.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
    
    assets.save_loaded_asset(rtg, only_if_is_dirty=False)
    unreal.log(f"Cleaned {rtg.get_name()}, after ops count: {ctrl.get_num_retarget_ops()}")

# Now retarget AL_StandardRig_Sword_Idle cleanly
rtg_ual = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin")
mesh_ual = assets.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_Standard")
mesh_target = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

anim_data = [assets.find_asset_data("/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle")]
inputs = unreal.IKRetargetBatchOperationInputs()
inputs.set_editor_property("assets_to_retarget", anim_data)
inputs.set_editor_property("ik_retarget_asset", rtg_ual)
inputs.set_editor_property("source_mesh", mesh_ual)
inputs.set_editor_property("target_mesh", mesh_target)
inputs.set_editor_property("target_path", "/Game/CastleDefender/Hero/Retargeted")
inputs.set_editor_property("overwrite_existing_files", True)

res = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
for r in res:
    obj = r.get_asset()
    if obj:
        assets.save_loaded_asset(obj, only_if_is_dirty=False)

unreal.log(f"Retargeted and saved {len(res)} assets.")

# Compare bone poses now
tgt_anim = assets.load_asset('/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle')
src_anim = assets.load_asset('/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle')
options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)
tgt_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(tgt_anim, 0.0, options)
src_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(src_anim, 0.0, options)

for b in ['pelvis', 'spine_01', 'spine_03', 'thigh_l', 'calf_l']:
    t_t = unreal.AnimPoseExtensions.get_bone_pose(tgt_pose, b, unreal.AnimPoseSpaces.WORLD)
    t_s = unreal.AnimPoseExtensions.get_bone_pose(src_pose, b, unreal.AnimPoseSpaces.WORLD)
    unreal.log(f"Cleaned [{b}] TGT pos=({t_t.translation.x:.1f}, {t_t.translation.y:.1f}, {t_t.translation.z:.1f}) rot=({t_t.rotation.rotator().pitch:.1f}, {t_t.rotation.rotator().yaw:.1f}, {t_t.rotation.rotator().roll:.1f})")
    unreal.log(f"        [{b}] SRC pos=({t_s.translation.x:.1f}, {t_s.translation.y:.1f}, {t_s.translation.z:.1f}) rot=({t_s.rotation.rotator().pitch:.1f}, {t_s.rotation.rotator().yaw:.1f}, {t_s.rotation.rotator().roll:.1f})")
