"""Creates Hero assets: DA_HeroClass_Warlord and BP_Hero_Warlord, and configures BP_SandboxGameMode.

Idempotent: can be run headless or in editor.
Run headless: Tools/create_hero_assets.bat
"""
import gc
import runpy
from pathlib import Path
import unreal

ROOT = "/Game/CastleDefender"
HERO_DIR = f"{ROOT}/Hero"
CORE_DIR = f"{ROOT}/Core"
INPUT_DIR = f"{CORE_DIR}/Input"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

# 1. Ensure directory exists
if not assets.does_directory_exist(HERO_DIR):
    assets.make_directory(HERO_DIR)

# 2. DA_HeroClass_Warlord
da_path = f"{HERO_DIR}/DA_HeroClass_Warlord"
hero_def_class = unreal.load_class(None, "/Script/CastleDefender.HeroClassDefinition")

new_definition = not assets.does_asset_exist(da_path)
if new_definition:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", hero_def_class)
    da = asset_tools.create_asset("DA_HeroClass_Warlord", HERO_DIR, hero_def_class, factory)
else:
    da = assets.load_asset(da_path)

if new_definition:
    da.set_editor_property("display_name", unreal.Text("Warlord"))
    da.set_editor_property("max_health", 200.0)

    movement = unreal.HeroMovementData()
    movement.set_editor_property("jog_speed", 450.0)
    movement.set_editor_property("sprint_speed", 700.0)
    movement.set_editor_property("rotation_rate_yaw", 720.0)
    da.set_editor_property("movement", movement)

    camera = unreal.HeroCameraData()
    camera.set_editor_property("target_arm_length", 400.0)
    camera.set_editor_property("socket_offset", unreal.Vector(0.0, 0.0, 0.0))
    camera.set_editor_property("enable_camera_lag", True)
    camera.set_editor_property("camera_lag_speed", 10.0)
    da.set_editor_property("camera", camera)

    stamina = unreal.StaminaConfig()
    stamina.set_editor_property("max", 100.0)
    stamina.set_editor_property("regen_delay", 0.8)
    stamina.set_editor_property("regen_rate", 30.0)
    stamina.set_editor_property("blocking_regen_multiplier", 0.5)
    stamina.set_editor_property("sprint_drain_per_second", 0.0)
    da.set_editor_property("stamina", stamina)
    da.set_editor_property("heavy_stamina_cost", 25.0)

# 2.1 Placeholder Mannequin content from the engine template (source/license: Placeholder/LICENSES.md).
placeholder = runpy.run_path(str(Path(__file__).with_name("create_hero_placeholder_content.py")))
placeholder["ensure_mannequin"]()
anim = placeholder["anim"]
skel = anim("Unarmed/MM_Idle").get_editor_property("skeleton")

A = unreal.HeroAction
LIGHT_CANCELS = [A.LIGHT, A.HEAVY, A.DODGE, A.BLOCK_START]
DODGE_CANCELS = [A.LIGHT, A.HEAVY, A.BLOCK_START]


def author_montage(name, clip, clip_end, duration, windows, reversed_clip=False, clip_start=0.0):
    """Creates the montage, or retargets a legacy Tutorial_Idle fixture to its Mannequin clip.

    Montages already on the Mannequin skeleton are left as authored. Returns (montage, changed).
    windows: (kind, start, duration, allowed actions) in montage time.
    """
    path = f"{HERO_DIR}/{name}"
    if assets.does_asset_exist(path):
        montage = assets.load_asset(path)
        if montage.get_editor_property("skeleton") == skel:
            return montage, False
    else:
        factory = unreal.AnimMontageFactory()
        factory.set_editor_property("target_skeleton", skel)
        factory.set_editor_property("source_animation", clip)
        montage = asset_tools.create_asset(name, HERO_DIR, unreal.AnimMontage, factory)
    assert unreal.HeroCombatLibrary.set_single_segment_montage_source(montage, clip, clip_start, clip_end, duration, reversed_clip)
    unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(montage)
    for kind, start, length, actions in windows:
        if kind == "hit":
            ok = unreal.HeroCombatLibrary.add_combat_hit_window_to_montage(montage, start, length)
        elif kind == "iframe":
            ok = unreal.HeroCombatLibrary.add_invulnerable_window_to_montage(montage, start, length)
        elif kind == "parry":
            ok = unreal.HeroCombatLibrary.add_parry_window_to_montage(montage, start, length)
        else:
            ok = unreal.HeroCombatLibrary.add_cancel_window_to_montage(montage, start, length, actions)
        assert ok, f"{name}: {kind} window rejected"
    assets.save_loaded_asset(montage, only_if_is_dirty=False)
    unreal.log(f"Authored {name} from {clip.get_name()}")
    return montage, True


# Light chain: hit windows follow the measured strike of each clip, Recovery cancels after.
light_specs = [
    ("AM_Warlord_Light_01", "Unarmed/Attack/MM_Attack_01", [("hit", 0.28, 0.17, None), ("cancel", 0.45, 0.40, LIGHT_CANCELS)]),
    ("AM_Warlord_Light_02", "Unarmed/Attack/MM_Attack_02", [("hit", 0.28, 0.18, None), ("cancel", 0.46, 0.39, LIGHT_CANCELS)]),
    ("AM_Warlord_Light_03", "Unarmed/Attack/MM_Attack_03", [("hit", 0.25, 0.35, None), ("cancel", 1.20, 0.35, [A.DODGE, A.BLOCK_START])]),
]
light_montages = []
for name, clip_path, windows in light_specs:
    clip = anim(clip_path)
    montage, _ = author_montage(name, clip, clip.get_play_length(), clip.get_play_length(), windows)
    light_montages.append(montage)

# Heavy: MM_ChargedAttack from 0.55 s (skips most of the 1 s hold) so the wind-up reads but stays ~0.5 s.
# The punch lands during the 150 cm root-motion lunge (clip 1.0-1.22 s); late recovery cancels into Dodge only.
charged = anim("Unarmed/Attack/MM_ChargedAttack")
heavy_start = 0.55
heavy_montage, _ = author_montage(
    "AM_Warlord_Heavy", charged, charged.get_play_length(), charged.get_play_length() - heavy_start,
    [("hit", 0.45, 0.22, None), ("cancel", 0.95, 0.30, [A.DODGE])], clip_start=heavy_start)
heavy_data = da.get_editor_property("heavy")
if not heavy_data.get_editor_property("montage"):
    heavy_data.set_editor_property("montage", heavy_montage)
    da.set_editor_property("heavy", heavy_data)

# Dodge: MM_Dash (motion ends ~0.73 s) fitted to 0.6 s; B plays it reversed. The template has no side steps,
# the lock-on authoring pass below replaces only those stock L/R sources with side-step clips.
dash = anim("Unarmed/Jump/MM_Dash")
dodge_windows = [("iframe", 0.10, 0.25, None), ("cancel", 0.40, 0.19, DODGE_CANCELS)]
dodge_data = da.get_editor_property("dodge")
dodge_changed = False
for suffix, field in [("F", "forward_montage"), ("B", "backward_montage"), ("L", "left_montage"), ("R", "right_montage")]:
    montage, changed = author_montage(f"AM_Warlord_Dodge_{suffix}", dash, 0.8, 0.6, dodge_windows, reversed_clip=(suffix == "B"))
    dodge_changed |= changed
    if not dodge_data.get_editor_property(field):
        dodge_data.set_editor_property(field, montage)
if dodge_changed:
    dodge_data.set_editor_property("root_motion_scale", 0.4)  # [TUNABLE] ~350 cm from the 885 cm dash clip
    # Newly authored stock L/R clips face their travel; the lock-on pass migrates them below.
    dodge_data.set_editor_property("side_clips_face_input", True)
da.set_editor_property("dodge", dodge_data)

react_data = da.get_editor_property("hit_react")
front = anim("Rifle/HitReact/MM_HitReact_Front_Med_01")
back = anim("Rifle/HitReact/MM_HitReact_Back_Med_01")
death = anim("Death/MM_Death_Front_01")
for name, clip, windows, field in [
        ("AM_Warlord_HitReact_F", front, [("cancel", 0.50, 0.25, [A.DODGE])], "front_montage"),
        ("AM_Warlord_HitReact_B", back, [("cancel", 0.55, 0.30, [A.DODGE])], "back_montage"),
        ("AM_Warlord_Death", death, [], "death_montage")]:
    montage, changed = author_montage(name, clip, clip.get_play_length(), clip.get_play_length(), windows)
    if changed and field == "death_montage":
        montage.set_editor_property("enable_auto_blend_out", False)  # hold the final pose until respawn
        assets.save_loaded_asset(montage, only_if_is_dirty=False)
    if not react_data.get_editor_property(field):
        react_data.set_editor_property(field, montage)
da.set_editor_property("hit_react", react_data)

# T-CMB-08: block reactions are presentation montages with no combat windows; Block itself is a held state.
block_data = da.get_editor_property("block")
for name, clip_path, field in [
        ("AM_Warlord_BlockHit", "Rifle/HitReact/MM_HitReact_Front_Lgt_01", "block_hit_montage"),
        ("AM_Warlord_BlockBreak", "Rifle/HitReact/MM_HitReact_Front_Hvy_01", "block_break_montage")]:
    clip = anim(clip_path)
    montage, _ = author_montage(name, clip, clip.get_play_length(), clip.get_play_length(), [])
    if not block_data.get_editor_property(field):
        block_data.set_editor_property(field, montage)
da.set_editor_property("block", block_data)

# T-CMB-09: separate high-risk press; authored 0.15 s active window, then 0.5 s whiff recovery.
parry = da.get_editor_property("parry")
parry_clip = anim("Unarmed/Attack/MM_Attack_01")
parry_montage, _ = author_montage("AM_Warlord_Parry", parry_clip, 0.70, 0.70,
                                  [("parry", 0.05, 0.15, None)])
if not parry.get_editor_property("montage"):
    parry.set_editor_property("montage", parry_montage)
    da.set_editor_property("parry", parry)

# Fill missing references without resetting attack numbers, states or resistance tuning.
attack_data_list = da.get_editor_property("light_chain")
if len(attack_data_list) != 3:
    raise RuntimeError("Existing LightChain must have three entries; refusing to replace custom data")
for i, attack in enumerate(attack_data_list):
    if not attack.get_editor_property("montage"):
        attack.set_editor_property("montage", light_montages[i])
da.set_editor_property("light_chain", attack_data_list)

# T-CMB-20: add only missing assist windows on the actual DA references; keep tuned notifies intact.
for attack in attack_data_list + [da.get_editor_property("heavy")]:
    montage = attack.get_editor_property("montage")
    assert unreal.HeroCombatLibrary.ensure_rotation_assist_window(montage), f"Invalid assist montage: {montage}"
    assets.save_loaded_asset(montage, only_if_is_dirty=True)

assets.save_loaded_asset(da, only_if_is_dirty=False)
unreal.log(f"Saved DA_HeroClass_Warlord at {da_path}")

# 3. BP_Hero_Warlord
bp_path = f"{HERO_DIR}/BP_Hero_Warlord"
hero_char_class = unreal.load_class(None, "/Script/CastleDefender.HeroCharacter")

ia_move = assets.load_asset(f"{INPUT_DIR}/IA_Move")
ia_look = assets.load_asset(f"{INPUT_DIR}/IA_Look")
ia_sprint = assets.load_asset(f"{INPUT_DIR}/IA_Sprint")
ia_light = assets.load_asset(f"{INPUT_DIR}/IA_LightAttack")
ia_heavy = assets.load_asset(f"{INPUT_DIR}/IA_HeavyAttack")
ia_dodge = assets.load_asset(f"{INPUT_DIR}/IA_Dodge")
ia_block = assets.load_asset(f"{INPUT_DIR}/IA_Block")

if not assets.does_asset_exist(bp_path):
    bp_factory = unreal.BlueprintFactory()
    bp_factory.set_editor_property("parent_class", hero_char_class)
    hero_bp = asset_tools.create_asset("BP_Hero_Warlord", HERO_DIR, None, bp_factory)
else:
    hero_bp = assets.load_asset(bp_path)

hero_cdo = unreal.get_default_object(hero_bp.generated_class())
hero_cdo.set_editor_property("hero_class_definition", da)
hero_cdo.set_editor_property("move_action", ia_move)
hero_cdo.set_editor_property("look_action", ia_look)
hero_cdo.set_editor_property("sprint_action", ia_sprint)
hero_cdo.set_editor_property("light_attack_action", ia_light)
hero_cdo.set_editor_property("heavy_attack_action", ia_heavy)
hero_cdo.set_editor_property("dodge_action", ia_dodge)
hero_cdo.set_editor_property("block_action", ia_block)

parry_path = f"{INPUT_DIR}/IA_Parry"
if not assets.does_asset_exist(parry_path):
    ia_parry = asset_tools.create_asset("IA_Parry", INPUT_DIR, unreal.InputAction, unreal.InputAction_Factory())
    ia_parry.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    assets.save_loaded_asset(ia_parry)
else:
    ia_parry = assets.load_asset(parry_path)
hero_cdo.set_editor_property("parry_action", ia_parry)
imc_combat = assets.load_asset(f"{INPUT_DIR}/IMC_Combat")
mapping_data = imc_combat.get_editor_property("default_key_mappings")
mapping_rows = list(mapping_data.get_editor_property("mappings"))
if not any(row.get_editor_property("action") == ia_parry for row in mapping_rows):
    key = unreal.Key()
    key.import_text("E")
    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property("action", ia_parry)
    mapping.set_editor_property("key", key)
    mapping_rows.append(mapping)
    mapping_data.set_editor_property("mappings", mapping_rows)
    imc_combat.set_editor_property("default_key_mappings", mapping_data)
    assets.save_loaded_asset(imc_combat)

# Distinct P0 Parry sound; preserve an already-authored feedback row.
feedback_table = assets.load_asset(f"{ROOT}/Feedback/DT_Feedback")
if feedback_table:
    import json
    rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(feedback_table))
    for row in rows:
        if row["Name"] == "Feedback.Combat.Parry" and "1kSineTonePing" in row.get("Sound", ""):
            sound_path = f"{ROOT}/Feedback/SFX_Parry"
            sound = assets.load_asset(sound_path) if assets.does_asset_exist(sound_path) else None
            if not sound:
                source = unreal.load_object(None, "/Engine/EditorSounds/Notifications/CompileSuccess.CompileSuccess")
                assert source, "Engine Parry placeholder source could not be loaded"
                sound = asset_tools.duplicate_asset("SFX_Parry", f"{ROOT}/Feedback", source)
                assets.save_loaded_asset(sound)
            assert sound, "Parry placeholder sound could not be created"
            row["Sound"] = sound.get_path_name()
            assert unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(feedback_table, json.dumps(rows))
            assets.save_loaded_asset(feedback_table)
            break

# 3.1 Visual assembly: Manny + ABP_Warlord (copy of the template ABP_Unarmed: locomotion + DefaultSlot), unarmed.
abp_path = f"{HERO_DIR}/ABP_Warlord"
if not assets.does_asset_exist(abp_path):
    abp = assets.duplicate_asset(f"{placeholder['MANNEQUIN']}/Anims/Unarmed/ABP_Unarmed", abp_path)
    unreal.get_default_object(abp.generated_class()).set_editor_property(
        "root_motion_mode", unreal.RootMotionMode.ROOT_MOTION_FROM_MONTAGES_ONLY)
    unreal.BlueprintEditorLibrary.compile_blueprint(abp)
    assets.save_loaded_asset(abp, only_if_is_dirty=False)
abp = assets.load_asset(abp_path)


def wire_foot_ik_alpha(abp):
    """Reparents to UHeroAnimInstance and drives the template foot IK Control Rig Alpha with FootIKAlpha.

    The template applies CR_Mannequin_FootIK after DefaultSlot, which pins the feet of attack montages to the ground.
    """
    bel, pins = unreal.BlueprintEditorLibrary, unreal.BlueprintGraphPinLibrary
    hero_anim = unreal.load_class(None, "/Script/CastleDefender.HeroAnimInstance")
    changed = bel.get_blueprint_parent_class(abp) != hero_anim
    if changed:
        bel.reparent_blueprint(abp, hero_anim)
    graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
    rigs = [n for n in graph.list_all_nodes() if n.get_class().get_name() == "AnimGraphNode_ControlRig"]
    if len(rigs) != 1:
        raise RuntimeError(f"ABP_Warlord: expected one foot IK Control Rig node, found {len(rigs)}")
    alpha = bel.find_input_pin(rigs[0], "Alpha")
    if not pins.is_valid(alpha):
        raise RuntimeError("ABP_Warlord: Control Rig node has no Alpha pin")
    if not pins.list_connected_pins(alpha):
        get = graph.add_get_member_variable_node("FootIKAlpha")
        if not get or not pins.try_create_connection(bel.find_output_pin(get, "FootIKAlpha"), alpha):
            raise RuntimeError("ABP_Warlord: could not connect FootIKAlpha to the Control Rig Alpha")
        rig_pos = bel.get_node_pos(rigs[0])
        bel.set_node_pos(get, unreal.IntPoint(rig_pos.x - 250, rig_pos.y + 120))
        changed = True
    if changed:
        if not bel.compile_blueprint(abp):
            raise RuntimeError("ABP_Warlord failed to compile after wiring foot IK")
        assets.save_loaded_asset(abp, only_if_is_dirty=False)


wire_foot_ik_alpha(abp)


def wire_guard_layer(abp):
    """T-CMB-08: inserts an upper-body guard layer (spine_01 up) before DefaultSlot, weighted by GuardAlpha.

    Placeholder pose: frame 0 of MM_Attack_01 (fists raised at chest height). Montages still override it.
    """
    bel, pins = unreal.BlueprintEditorLibrary, unreal.BlueprintGraphPinLibrary
    graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
    nodes = graph.list_all_nodes()
    if any(n.get_class().get_name() == "AnimGraphNode_LayeredBoneBlend" for n in nodes):
        return
    slots = [n for n in nodes if n.get_class().get_name() == "AnimGraphNode_Slot"]
    if len(slots) != 1:
        raise RuntimeError(f"ABP_Warlord: expected one Slot node, found {len(slots)}")
    slot_source = bel.find_input_pin(slots[0], "Source")
    upstream = pins.list_connected_pins(slot_source)
    if len(upstream) != 1:
        raise RuntimeError("ABP_Warlord: DefaultSlot Source must have exactly one input")
    slot_pos = bel.get_node_pos(slots[0])
    layered = graph.create_node_from_name("Animation|Blends|Layeredblendperbone",
                                          unreal.Vector2D(slot_pos.x - 40, slot_pos.y + 260), [])
    guard = graph.create_node_from_name("Animation|Sequences|Evaluate'MM_Attack_01'",
                                        unreal.Vector2D(slot_pos.x - 360, slot_pos.y + 360), [])
    alpha = graph.add_get_member_variable_node("GuardAlpha")
    if not layered or not guard or not alpha:
        raise RuntimeError("ABP_Warlord: could not create the guard layer nodes")
    bel.set_node_pos(alpha, unreal.IntPoint(slot_pos.x - 300, slot_pos.y + 480))

    inner = layered.get_editor_property("node")
    branch = unreal.BranchFilter()
    branch.set_editor_property("bone_name", "spine_01")
    branch.set_editor_property("blend_depth", 0)
    layer = unreal.InputBlendPose()
    layer.set_editor_property("branch_filters", [branch])
    inner.set_editor_property("layer_setup", [layer])
    inner.set_editor_property("mesh_space_rotation_blend", True)
    layered.set_editor_property("node", inner)

    source = upstream[0]
    pins.break_pin_links(slot_source)
    links = [
        (source, bel.find_input_pin(layered, "BasePose")),
        (bel.find_output_pin(guard, "Pose"), bel.find_input_pin(layered, "BlendPoses_0")),
        (bel.find_output_pin(alpha, "GuardAlpha"), bel.find_input_pin(layered, "BlendWeights_0")),
        (bel.find_output_pin(layered, "Pose"), slot_source),
    ]
    for out_pin, in_pin in links:
        if not pins.try_create_connection(out_pin, in_pin):
            raise RuntimeError(f"ABP_Warlord: could not connect {pins.get_pin_name(out_pin)} -> {pins.get_pin_name(in_pin)}")
    if not bel.compile_blueprint(abp):
        raise RuntimeError("ABP_Warlord failed to compile after adding the guard layer")
    assets.save_loaded_asset(abp, only_if_is_dirty=False)
    unreal.log("ABP_Warlord: guard layer wired (spine_01, GuardAlpha)")


wire_guard_layer(abp)

# T-CMB-10: lock-on input, strafe locomotion, side-step dodges and marker overlay.
runpy.run_path(str(Path(__file__).with_name("create_lock_on_content.py")))["setup_lock_on"](hero_bp, abp, da, anim)

mesh = hero_cdo.get_editor_property("mesh")
if not mesh.get_skeletal_mesh_asset():
    half_height = hero_cdo.get_editor_property("capsule_component").get_unscaled_capsule_half_height()
    mesh.set_skeletal_mesh_asset(assets.load_asset(f"{placeholder['MANNEQUIN']}/Meshes/SKM_Manny_Simple"))
    mesh.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -half_height))
    mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
if not mesh.get_editor_property("anim_class"):
    mesh.set_editor_property("anim_class", abp.generated_class())
# Unarmed demo: no weapon mesh, so MeleeTraceComponent sweeps its fallback arc in front of the hero
# and no bone sockets drive hits; the engine default tick option is enough.
mesh.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE)
weapon = hero_cdo.get_editor_property("weapon_mesh")
weapon.set_editor_property("static_mesh", None)
weapon.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 0.0))
weapon.set_editor_property("relative_scale3d", unreal.Vector(1.0, 1.0, 1.0))

unreal.BlueprintEditorLibrary.compile_blueprint(hero_bp)
assets.save_loaded_asset(hero_bp, only_if_is_dirty=False)
unreal.log(f"Saved BP_Hero_Warlord at {bp_path}")

hero_pawn_class = hero_bp.generated_class()

# 4. Wire BP_Hero_Warlord as default pawn in BP_SandboxGameMode
gm_path = f"{CORE_DIR}/BP_SandboxGameMode"
if assets.does_asset_exist(gm_path):
    gm_bp = assets.load_asset(gm_path)
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    gm_cdo.set_editor_property("default_pawn_class", hero_pawn_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(gm_bp)
    runpy.run_path(str(Path(__file__).with_name("create_sandbox_respawn.py")))["wire_sandbox_respawn"](gm_bp)
    assets.save_loaded_asset(gm_bp, only_if_is_dirty=False)
    unreal.log("Updated BP_SandboxGameMode default_pawn_class to BP_Hero_Warlord")

print("Hero assets created and configured successfully.")
