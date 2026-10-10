import unreal

mm_idle = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Placeholder/Mannequins/Anims/Unarmed/MM_Idle')
sword_idle = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle')
src_sword_idle = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle')

options = unreal.AnimPoseEvaluationOptions()
options.set_editor_property('extract_root_motion', False)

p_mm = unreal.AnimPoseExtensions.get_anim_pose_at_time(mm_idle, 0.0, options)
p_sw = unreal.AnimPoseExtensions.get_anim_pose_at_time(sword_idle, 0.0, options)
p_src = unreal.AnimPoseExtensions.get_anim_pose_at_time(src_sword_idle, 0.0, options)

bones = ['pelvis', 'spine_01', 'spine_02', 'spine_03', 'thigh_l', 'calf_l', 'upperarm_l', 'upperarm_r']

unreal.log("=== BONE POSES AT T=0 (WORLD) ===")
for b in bones:
    t_mm = unreal.AnimPoseExtensions.get_bone_pose(p_mm, b, unreal.AnimPoseSpaces.WORLD)
    t_sw = unreal.AnimPoseExtensions.get_bone_pose(p_sw, b, unreal.AnimPoseSpaces.WORLD)
    t_src = unreal.AnimPoseExtensions.get_bone_pose(p_src, b, unreal.AnimPoseSpaces.WORLD)
    unreal.log(f"[{b}]")
    unreal.log(f"   MM_Idle:       pos=({t_mm.translation.x:.1f}, {t_mm.translation.y:.1f}, {t_mm.translation.z:.1f}) rot=({t_mm.rotation.rotator().pitch:.1f}, {t_mm.rotation.rotator().yaw:.1f}, {t_mm.rotation.rotator().roll:.1f})")
    unreal.log(f"   Retargeted_Sw: pos=({t_sw.translation.x:.1f}, {t_sw.translation.y:.1f}, {t_sw.translation.z:.1f}) rot=({t_sw.rotation.rotator().pitch:.1f}, {t_sw.rotation.rotator().yaw:.1f}, {t_sw.rotation.rotator().roll:.1f})")
    unreal.log(f"   Source_Sw:     pos=({t_src.translation.x:.1f}, {t_src.translation.y:.1f}, {t_src.translation.z:.1f}) rot=({t_src.rotation.rotator().pitch:.1f}, {t_src.rotation.rotator().yaw:.1f}, {t_src.rotation.rotator().roll:.1f})")
