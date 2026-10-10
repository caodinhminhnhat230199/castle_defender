import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
bel = unreal.BlueprintEditorLibrary

TARGET_DIR = "/Game/CastleDefender/Hero/Retargeted"
HERO_DIR = "/Game/CastleDefender/Hero"

unreal.log("=== VERIFYING RETARGETED ASSETS ===")
check_list = [
    ("UAL2_StandardSword_Regular_A", "hand_r"),
    ("UAL2_StandardSword_Regular_B", "hand_r"),
    ("UAL2_StandardSword_Regular_C", "hand_r"),
    ("UAL2_StandardSword_Dash_RM", "hand_r"),
    ("UAL2_StandardShield_Dash_RM", "pelvis"),
    ("UAL2_StandardHit_Knockback", "pelvis"),
    ("AL_StandardRig_Sword_Idle", "hand_r"),
    ("AL_StandardRig_Jump_Start", "pelvis"),
    ("AL_StandardRig_Jump_Loop", "pelvis"),
    ("AL_StandardRig_Jump_Land", "pelvis"),
]

for name, bone in check_list:
    seq = assets.load_asset(f"{TARGET_DIR}/{name}")
    if not seq:
        unreal.log_error(f"Asset missing: {name}")
        continue
    length = seq.get_editor_property("sequence_length")
    keys = seq.get_editor_property("number_of_sampled_keys")
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(seq, bone, 0.0, False)
    t_mid = unreal.AnimationLibrary.get_bone_pose_for_time(seq, bone, length * 0.5, False)
    r0 = t0.rotation.rotator()
    r1 = t_mid.rotation.rotator()
    rot_delta = abs(r1.pitch - r0.pitch) + abs(r1.yaw - r0.yaw) + abs(r1.roll - r0.roll)
    p0 = t0.translation
    p1 = t_mid.translation
    pos_delta = (p1 - p0).length()
    unreal.log(f"SEQ '{name}': len={length:.3f}s, keys={keys}, bone={bone}, pos_delta={pos_delta:.2f}, rot_delta={rot_delta:.2f} deg")

unreal.log("=== VERIFYING MONTAGES ===")
montages = [
    "AM_Warlord_Light_01",
    "AM_Warlord_Light_02",
    "AM_Warlord_Light_03",
    "AM_Warlord_Heavy",
    "AM_Warlord_Dodge_F",
    "AM_Warlord_Death",
]
for mname in montages:
    m = assets.load_asset(f"{HERO_DIR}/{mname}")
    if m:
        seq_len = m.get_play_length()
        unreal.log(f"Montage '{mname}': play_length={seq_len:.3f}s")

unreal.log("=== VERIFYING ABP_Warlord NODES ===")
abp = assets.load_asset(f"{HERO_DIR}/ABP_Warlord")
for g in bel.list_graphs(abp):
    gname = g.get_name()
    if gname in ["Idle", "Jump", "Fall Loop", "Land"]:
        editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
        for n in editor.list_all_nodes():
            if n.get_class().get_name() == "AnimGraphNode_SequencePlayer":
                struct_prop = n.get_editor_property("node")
                seq_obj = struct_prop.get_editor_property("sequence")
                unreal.log(f"ABP Graph '{gname}' -> {seq_obj.get_name() if seq_obj else 'None'}")
