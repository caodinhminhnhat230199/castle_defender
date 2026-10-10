import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

rtg_ual = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin")
mesh_ual = assets.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_Standard")
mesh_target = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

anim_data = [assets.find_asset_data("/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Idle_Loop")]
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

unreal.log(f"Retargeted {len(res)} assets.")

retargeted = assets.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Idle_Loop")
if retargeted:
    options = unreal.AnimPoseEvaluationOptions()
    options.set_editor_property('extract_root_motion', False)
    pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(retargeted, 0.0, options)
    for b in ['pelvis', 'spine_01', 'spine_03', 'thigh_l', 'calf_l', 'foot_l', 'upperarm_l', 'upperarm_r']:
        t = unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD)
        unreal.log(f"Retargeted Idle_Loop [{b}]: Z={t.translation.z:.1f} rot=({t.rotation.rotator().pitch:.1f}, {t.rotation.rotator().yaw:.1f}, {t.rotation.rotator().roll:.1f})")
