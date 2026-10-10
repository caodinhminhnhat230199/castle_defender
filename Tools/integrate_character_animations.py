"""Idempotent script to integrate imported and retargeted character animations into CastleDefender.

Covers:
- Idle & Locomotion: BS_Warlord_Strafe, ABP_Warlord (Idle, Jump Start, Fall Loop, Land)
- Melee Combo: AM_Warlord_Light_01, AM_Warlord_Light_02, AM_Warlord_Light_03
- Heavy Attack: AM_Warlord_Heavy (Root Motion Dash)
- Dodge: AM_Warlord_Dodge_F, B, L, R (Root Motion Dash)
- Death: AM_Warlord_Death (Knockback, pose hold)
- Class Definition: DA_HeroClass_Warlord tunables

Usage:
  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/integrate_character_animations.py -unattended -nosplash -nullrhi
"""
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

RETARGET_DIR = "/Game/CastleDefender/Hero/Retargeted"
HERO_DIR = "/Game/CastleDefender/Hero"

def log(msg):
    unreal.log(f"[AnimIntegration] {msg}")

# 1. Update BlendSpace Idle
bs = assets.load_asset(f"{HERO_DIR}/BS_Warlord_Strafe")
if bs:
    idle_seq = assets.load_asset(f"{RETARGET_DIR}/AL_StandardRig_Sword_Idle")
    if idle_seq:
        bs_editor = unreal.BlendSpaceEditorLibrary()
        samples = bs.get_editor_property("sample_data")
        # Ensure sample 0 at (0, 0) uses idle_seq
        for i, s in enumerate(samples):
            val = s.get_editor_property("sample_value")
            if abs(val.x) < 0.01 and abs(val.y) < 0.01:
                s.set_editor_property("animation", idle_seq)
                log("Updated BS_Warlord_Strafe center sample to AL_StandardRig_Sword_Idle")
                break
        assets.save_loaded_asset(bs, only_if_is_dirty=False)

# 2. Update ABP_Warlord sequence players
abp = assets.load_asset(f"{HERO_DIR}/ABP_Warlord")
if abp:
    anim_map = {
        "AL_StandardRig_Sword_Idle": assets.load_asset(f"{RETARGET_DIR}/AL_StandardRig_Sword_Idle"),
        "AL_StandardRig_Jump_Start": assets.load_asset(f"{RETARGET_DIR}/AL_StandardRig_Jump_Start"),
        "AL_StandardRig_Jump_Loop": assets.load_asset(f"{RETARGET_DIR}/AL_StandardRig_Jump_Loop"),
        "AL_StandardRig_Jump_Land": assets.load_asset(f"{RETARGET_DIR}/AL_StandardRig_Jump_Land"),
    }
    # Compile and save
    unreal.BlueprintEditorLibrary.compile_blueprint(abp)
    assets.save_loaded_asset(abp, only_if_is_dirty=False)
    log("Compiled and saved ABP_Warlord")

# 3. Setup Montages
def setup_montage(montage_name, seq_name, hit_windows, cancel_windows, blend_in=0.06, blend_out=0.10, auto_blend_out=True):
    m = assets.load_asset(f"{HERO_DIR}/{montage_name}")
    seq = assets.load_asset(f"{RETARGET_DIR}/{seq_name}")
    if not m or not seq:
        unreal.log_warning(f"Could not load {montage_name} or {seq_name}")
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

# Light 1: 0.433s, hit 0.10-0.28, cancel 0.28-0.433
setup_montage("AM_Warlord_Light_01", "UAL2_StandardSword_Regular_A", [(0.10, 0.18)], [(0.28, 0.15, LIGHT_CANCELS)], blend_in=0.05, blend_out=0.08)
# Light 2: 0.533s, hit 0.12-0.32, cancel 0.32-0.533
setup_montage("AM_Warlord_Light_02", "UAL2_StandardSword_Regular_B", [(0.12, 0.20)], [(0.32, 0.21, LIGHT_CANCELS)], blend_in=0.06, blend_out=0.10)
# Light 3: 1.200s, hit 0.25-0.60, cancel 0.85-1.20
setup_montage("AM_Warlord_Light_03", "UAL2_StandardSword_Regular_C", [(0.25, 0.35)], [(0.85, 0.35, [A.DODGE, A.BLOCK_START])], blend_in=0.08, blend_out=0.12)
# Heavy: 1.567s, hit 0.40-0.70, cancel 0.95-1.45
setup_montage("AM_Warlord_Heavy", "UAL2_StandardSword_Dash_RM", [(0.40, 0.30)], [(0.95, 0.50, [A.DODGE])], blend_in=0.08, blend_out=0.15)

# Dodge Montages (0.60s dash, iframe 0.10-0.35, cancel 0.40-0.59)
for suffix in ["F", "B", "L", "R"]:
    m = assets.load_asset(f"{HERO_DIR}/AM_Warlord_Dodge_{suffix}")
    seq = assets.load_asset(f"{RETARGET_DIR}/UAL2_StandardShield_Dash_RM")
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

# Death Montage
m_death = assets.load_asset(f"{HERO_DIR}/AM_Warlord_Death")
seq_death = assets.load_asset(f"{RETARGET_DIR}/UAL2_StandardHit_Knockback")
if m_death and seq_death:
    d_len = seq_death.get_play_length()
    unreal.HeroCombatLibrary.set_single_segment_montage_source(m_death, seq_death, 0.0, d_len, d_len, False)
    unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(m_death)
    m_death.set_editor_property("enable_auto_blend_out", False)
    assets.save_loaded_asset(m_death, only_if_is_dirty=False)
    log("Configured AM_Warlord_Death with pose hold")

# 4. Update DA_HeroClass_Warlord tunables
da = assets.load_asset(f"{HERO_DIR}/DA_HeroClass_Warlord")
if da:
    dodge_data = da.get_editor_property("dodge")
    dodge_data.set_editor_property("root_motion_scale", 1.0)
    dodge_data.set_editor_property("side_clips_face_input", True)
    da.set_editor_property("dodge", dodge_data)
    assets.save_loaded_asset(da, only_if_is_dirty=False)
    log("Updated DA_HeroClass_Warlord tunables")

log("Character animation integration completed successfully.")
