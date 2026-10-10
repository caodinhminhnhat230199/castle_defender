import unreal

skm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
if skm:
    # check skeleton
    skel = skm.get_editor_property("skeleton")
    unreal.log(f"Skeleton: {skel}")
    # check sockets on skeleton
    sockets = skel.get_editor_property("sockets") if hasattr(skel, "sockets") else []
    unreal.log(f"Skeleton sockets: {[s.get_editor_property('socket_name') for s in sockets]}")

# Also check an animation sequence to see hand_r bone transform
anim = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle")
if anim:
    # evaluate bone transform at frame 0
    t = anim.get_bone_pose_for_frame("hand_r", 0, False)
    unreal.log(f"Sword_Idle hand_r pose: loc={t.translation}, rot={t.rotation.rotator()}")

