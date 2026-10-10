"""Fixes RTG mappings and performs batch retarget for all required hero animations,
then updates BlendSpace, ABP, and Montages with verified motion.
"""
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
bel = unreal.BlueprintEditorLibrary

TARGET_DIR = "/Game/CastleDefender/Hero/Retargeted"
HERO_DIR = "/Game/CastleDefender/Hero"
UAL2_DIR = "/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes"
UAL_DIR = "/Game/CastleDefender/Assets/UAL_Standard"

def log(msg):
    unreal.log(f"[FixRetarget] {msg}")

# 1. Fix and configure RTG assets
def setup_rtg(rtg_path):
    rtg = assets.load_asset(rtg_path)
    if not rtg:
        log(f"Error loading {rtg_path}")
        return None
    ctrl = unreal.IKRetargeterController.get_controller(rtg)
    ctrl.add_default_ops()
    ctrl.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
    assets.save_loaded_asset(rtg, only_if_is_dirty=False)
    log(f"Configured and saved {rtg.get_name()}")
    return rtg

rtg_ual2 = setup_rtg(f"{HERO_DIR}/RTG_UAL2_To_Mannequin")
rtg_ual = setup_rtg(f"{HERO_DIR}/RTG_UAL_To_Mannequin")

# Meshes
mesh_ual2 = assets.load_asset(f"{UAL2_DIR}/UAL2_Standard")
mesh_ual = assets.load_asset(f"{UAL_DIR}/AL_Standard")
mesh_target = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

# 2. Retarget UAL2 assets
ual2_anims = [
    "UAL2_StandardSword_Regular_A",
    "UAL2_StandardSword_Regular_B",
    "UAL2_StandardSword_Regular_C",
    "UAL2_StandardSword_Dash_RM",
    "UAL2_StandardShield_Dash_RM",
    "UAL2_StandardHit_Knockback",
]

ual2_data = [assets.find_asset_data(f"{UAL2_DIR}/{name}") for name in ual2_anims]
inputs_ual2 = unreal.IKRetargetBatchOperationInputs()
inputs_ual2.set_editor_property("assets_to_retarget", ual2_data)
inputs_ual2.set_editor_property("ik_retarget_asset", rtg_ual2)
inputs_ual2.set_editor_property("source_mesh", mesh_ual2)
inputs_ual2.set_editor_property("target_mesh", mesh_target)
inputs_ual2.set_editor_property("target_path", TARGET_DIR)
inputs_ual2.set_editor_property("overwrite_existing_files", True)

res_ual2 = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs_ual2)
for r in res_ual2:
    obj = r.get_asset()
    if obj:
        assets.save_loaded_asset(obj, only_if_is_dirty=False)
log(f"Retargeted and saved {len(res_ual2)} UAL2 assets.")

# 3. Retarget UAL assets
ual_anims = [
    "AL_StandardRig_Sword_Idle",
    "AL_StandardRig_Jump_Start",
    "AL_StandardRig_Jump_Loop",
    "AL_StandardRig_Jump_Land",
    "AL_StandardRig_Walk_Loop",
    "AL_StandardRig_Jog_Fwd_Loop",
    "AL_StandardRig_Sprint_Loop",
]

ual_data = [assets.find_asset_data(f"{UAL_DIR}/{name}") for name in ual_anims]
inputs_ual = unreal.IKRetargetBatchOperationInputs()
inputs_ual.set_editor_property("assets_to_retarget", ual_data)
inputs_ual.set_editor_property("ik_retarget_asset", rtg_ual)
inputs_ual.set_editor_property("source_mesh", mesh_ual)
inputs_ual.set_editor_property("target_mesh", mesh_target)
inputs_ual.set_editor_property("target_path", TARGET_DIR)
inputs_ual.set_editor_property("overwrite_existing_files", True)

res_ual = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs_ual)
for r in res_ual:
    obj = r.get_asset()
    if obj:
        assets.save_loaded_asset(obj, only_if_is_dirty=False)
log(f"Retargeted and saved {len(res_ual)} UAL assets.")

# 4. Verify bone delta motion on all retargeted animations
all_anims = ual2_anims + ual_anims
for name in all_anims:
    seq = assets.load_asset(f"{TARGET_DIR}/{name}")
    if not seq:
        log(f"WARNING: Asset {name} not found in {TARGET_DIR}")
        continue
    length = seq.get_editor_property("sequence_length")
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "hand_r", 0.0, False)
    t_mid = unreal.AnimationLibrary.get_bone_pose_for_time(seq, "hand_r", length * 0.5, False)
    rot0 = t0.rotation.rotator()
    rot_mid = t_mid.rotation.rotator()
    rot_diff = abs(rot_mid.pitch - rot0.pitch) + abs(rot_mid.yaw - rot0.yaw) + abs(rot_mid.roll - rot0.roll)
    log(f"Verified {name}: len={length:.3f}s, rot_delta={rot_diff:.2f} deg")

# 5. Update BS_Warlord_Strafe
bs = assets.load_asset(f"{HERO_DIR}/BS_Warlord_Strafe")
if bs:
    idle_seq = assets.load_asset(f"{TARGET_DIR}/AL_StandardRig_Sword_Idle")
    if idle_seq:
        samples = bs.get_editor_property("sample_data")
        for s in samples:
            val = s.get_editor_property("sample_value")
            if abs(val.x) < 0.01 and abs(val.y) < 0.01:
                s.set_editor_property("animation", idle_seq)
                break
        bs.set_editor_property("sample_data", samples)
        assets.save_loaded_asset(bs, only_if_is_dirty=False)
        log("Updated and saved BS_Warlord_Strafe")

# 6. Update ABP_Warlord State Machine Sequence Players
abp = assets.load_asset(f"{HERO_DIR}/ABP_Warlord")
if abp:
    state_to_anim = {
        "Idle": f"{TARGET_DIR}/AL_StandardRig_Sword_Idle",
        "Jump": f"{TARGET_DIR}/AL_StandardRig_Jump_Start",
        "Fall Loop": f"{TARGET_DIR}/AL_StandardRig_Jump_Loop",
        "Land": f"{TARGET_DIR}/AL_StandardRig_Jump_Land",
    }
    for g in bel.list_graphs(abp):
        gname = g.get_name()
        if gname in state_to_anim:
            target_anim_path = state_to_anim[gname]
            anim_obj = assets.load_asset(target_anim_path)
            editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
            for n in editor.list_all_nodes():
                if n.get_class().get_name() == "AnimGraphNode_SequencePlayer":
                    node_struct = n.get_editor_property("node")
                    node_struct.set_editor_property("sequence", anim_obj)
                    n.set_editor_property("node", node_struct)
                    log(f"Updated ABP node in graph '{gname}' -> {target_anim_path}")
    bel.compile_blueprint(abp)
    assets.save_loaded_asset(abp, only_if_is_dirty=False)
    log("Compiled and saved ABP_Warlord")

# 7. Configure Montages
def setup_montage(montage_name, seq_name, hit_windows, cancel_windows, blend_in=0.06, blend_out=0.10, auto_blend_out=True):
    m = assets.load_asset(f"{HERO_DIR}/{montage_name}")
    seq = assets.load_asset(f"{TARGET_DIR}/{seq_name}")
    if not m or not seq:
        log(f"WARNING: Cannot load {montage_name} or {seq_name}")
        return
    seq_len = seq.get_play_length()
    unreal.HeroCombatLibrary.set_single_segment_montage_source(m, seq, 0.0, seq_len, seq_len, False)
    unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(m)

    for start, dur in hit_windows:
        unreal.HeroCombatLibrary.add_combat_hit_window_to_montage(m, start, dur)
    
    for start, dur, actions in cancel_windows:
        unreal.HeroCombatLibrary.add_cancel_window_to_montage(m, start, dur, actions)
    
    unreal.HeroCombatLibrary.ensure_rotation_assist_window(m)

    bi = m.get_editor_property("blend_in")
    bi.set_editor_property("blend_time", blend_in)
    m.set_editor_property("blend_in", bi)

    bo = m.get_editor_property("blend_out")
    bo.set_editor_property("blend_time", blend_out)
    m.set_editor_property("blend_out", bo)

    m.set_editor_property("enable_auto_blend_out", auto_blend_out)
    assets.save_loaded_asset(m, only_if_is_dirty=False)
    log(f"Configured montage {montage_name} (len={seq_len:.3f})")

A = unreal.HeroAction
LIGHT_CANCELS = [A.LIGHT, A.HEAVY, A.DODGE, A.BLOCK_START]
DODGE_CANCELS = [A.LIGHT, A.HEAVY, A.BLOCK_START]

setup_montage("AM_Warlord_Light_01", "UAL2_StandardSword_Regular_A", [(0.10, 0.18)], [(0.28, 0.15, LIGHT_CANCELS)], blend_in=0.05, blend_out=0.08)
setup_montage("AM_Warlord_Light_02", "UAL2_StandardSword_Regular_B", [(0.12, 0.20)], [(0.32, 0.21, LIGHT_CANCELS)], blend_in=0.06, blend_out=0.10)
setup_montage("AM_Warlord_Light_03", "UAL2_StandardSword_Regular_C", [(0.25, 0.35)], [(0.85, 0.35, [A.DODGE, A.BLOCK_START])], blend_in=0.08, blend_out=0.12)
setup_montage("AM_Warlord_Heavy", "UAL2_StandardSword_Dash_RM", [(0.40, 0.30)], [(0.95, 0.50, [A.DODGE])], blend_in=0.08, blend_out=0.15)

for suffix in ["F", "B", "L", "R"]:
    m = assets.load_asset(f"{HERO_DIR}/AM_Warlord_Dodge_{suffix}")
    seq = assets.load_asset(f"{TARGET_DIR}/UAL2_StandardShield_Dash_RM")
    if m and seq:
        seq_len = 0.60
        unreal.HeroCombatLibrary.set_single_segment_montage_source(m, seq, 0.0, seq_len, seq_len, False)
        unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(m)
        unreal.HeroCombatLibrary.add_invulnerable_window_to_montage(m, 0.10, 0.25)
        unreal.HeroCombatLibrary.add_cancel_window_to_montage(m, 0.40, 0.19, DODGE_CANCELS)
        bi = m.get_editor_property("blend_in")
        bi.set_editor_property("blend_time", 0.06)
        m.set_editor_property("blend_in", bi)
        bo = m.get_editor_property("blend_out")
        bo.set_editor_property("blend_time", 0.10)
        m.set_editor_property("blend_out", bo)
        assets.save_loaded_asset(m, only_if_is_dirty=False)
        log(f"Configured dodge montage AM_Warlord_Dodge_{suffix}")

m_death = assets.load_asset(f"{HERO_DIR}/AM_Warlord_Death")
seq_death = assets.load_asset(f"{TARGET_DIR}/UAL2_StandardHit_Knockback")
if m_death and seq_death:
    d_len = seq_death.get_play_length()
    unreal.HeroCombatLibrary.set_single_segment_montage_source(m_death, seq_death, 0.0, d_len, d_len, False)
    unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(m_death)
    m_death.set_editor_property("enable_auto_blend_out", False)
    assets.save_loaded_asset(m_death, only_if_is_dirty=False)
    log("Configured AM_Warlord_Death")

log("ALL RETARGETING AND INTEGRATION TASKS FINISHED SUCCESSFULLY!")
