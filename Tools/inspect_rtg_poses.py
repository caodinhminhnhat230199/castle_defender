import unreal

rtg = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin')
ctrl = unreal.IKRetargeterController.get_controller(rtg)

src_mesh = ctrl.get_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE)
tgt_mesh = ctrl.get_preview_mesh(unreal.RetargetSourceOrTarget.TARGET)

unreal.log(f"Source Preview Mesh: {src_mesh}")
unreal.log(f"Target Preview Mesh: {tgt_mesh}")

poses_src = ctrl.get_retarget_pose_names(unreal.RetargetSourceOrTarget.SOURCE)
unreal.log(f"Source retarget poses: {poses_src}")
poses_tgt = ctrl.get_retarget_pose_names(unreal.RetargetSourceOrTarget.TARGET)
unreal.log(f"Target retarget poses: {poses_tgt}")
unreal.log(f"Current Source retarget pose: {ctrl.get_current_retarget_pose_name(unreal.RetargetSourceOrTarget.SOURCE)}")
unreal.log(f"Current Target retarget pose: {ctrl.get_current_retarget_pose_name(unreal.RetargetSourceOrTarget.TARGET)}")

# Inspect bone orientations of source vs target preview mesh
if src_mesh and tgt_mesh:
    src_skel = src_mesh.get_editor_property('skeleton')
    tgt_skel = tgt_mesh.get_editor_property('skeleton')
    unreal.log(f"Source Skeleton: {src_skel}")
    unreal.log(f"Target Skeleton: {tgt_skel}")

    # Inspect retarget pose delta
    retarget_pose = ctrl.get_current_retarget_pose(unreal.RetargetSourceOrTarget.TARGET)
    unreal.log(f"Target Retarget Pose Object: {retarget_pose}")
