import unreal

ual2_anim = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Regular_A')
ual2_tgt = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/Retargeted/UAL2_StandardSword_Regular_A')

options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)
p_src = unreal.AnimPoseExtensions.get_anim_pose_at_time(ual2_anim, 0.0, options)
p_tgt = unreal.AnimPoseExtensions.get_anim_pose_at_time(ual2_tgt, 0.0, options)

unreal.log("=== UAL2 Sword Attack A Poses ===")
for b in ['pelvis', 'spine_01', 'spine_03']:
    t_s = unreal.AnimPoseExtensions.get_bone_pose(p_src, b, unreal.AnimPoseSpaces.WORLD)
    t_t = unreal.AnimPoseExtensions.get_bone_pose(p_tgt, b, unreal.AnimPoseSpaces.WORLD)
    unreal.log(f"UAL2 {b}: SRC rot=({t_s.rotation.rotator().pitch:.1f}, {t_s.rotation.rotator().yaw:.1f}, {t_s.rotation.rotator().roll:.1f})")
    unreal.log(f"UAL2 {b}: TGT rot=({t_t.rotation.rotator().pitch:.1f}, {t_t.rotation.rotator().yaw:.1f}, {t_t.rotation.rotator().roll:.1f})")
