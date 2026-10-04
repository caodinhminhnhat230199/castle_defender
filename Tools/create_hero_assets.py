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

# 2.1 Montages for Warlord Light Chain
reg = unreal.AssetRegistryHelpers.get_asset_registry()
reg.scan_paths_synchronous(["/Engine", "/Game"])

anim_path = "/Engine/Tutorial/SubEditors/TutorialAssets/Character/Tutorial_Idle"
anim_seq = assets.load_asset(anim_path)
skel = anim_seq.get_editor_property("skeleton") if anim_seq else None

light_montages = []
for idx, m_name in enumerate(["AM_Warlord_Light_01", "AM_Warlord_Light_02", "AM_Warlord_Light_03"]):
    m_path = f"{HERO_DIR}/{m_name}"
    new_montage = not assets.does_asset_exist(m_path)
    if new_montage:
        montage_factory = unreal.AnimMontageFactory()
        if skel:
            montage_factory.set_editor_property("target_skeleton", skel)
            montage_factory.set_editor_property("source_animation", anim_seq)
        m = asset_tools.create_asset(m_name, HERO_DIR, unreal.AnimMontage, montage_factory)
    else:
        m = assets.load_asset(m_path)

    if new_montage:
        # Setup notifies: 1 hit window + cancel window
        if idx < 2:
            # Light 1 and 2: Hit 0.15s (0.2s duration), Cancel 0.35s (0.5s duration) allows Light, Heavy, Dodge, Block
            unreal.HeroCombatLibrary.add_combat_hit_window_to_montage(m, 0.15, 0.2)
            unreal.HeroCombatLibrary.add_cancel_window_to_montage(m, 0.35, 0.5, [
                unreal.HeroAction.LIGHT,
                unreal.HeroAction.HEAVY,
                unreal.HeroAction.DODGE,
                unreal.HeroAction.BLOCK_START
            ])
        else:
            # Light 3: Hit 0.2s (0.25s duration), Cancel 0.45s (0.5s duration) allows Dodge, Block
            unreal.HeroCombatLibrary.add_combat_hit_window_to_montage(m, 0.2, 0.25)
            unreal.HeroCombatLibrary.add_cancel_window_to_montage(m, 0.45, 0.5, [
                unreal.HeroAction.DODGE,
                unreal.HeroAction.BLOCK_START
            ])

    assets.save_loaded_asset(m, only_if_is_dirty=False)
    light_montages.append(m)
    unreal.log(f"Saved {m_name} at {m_path}")

# Dodge timing fixtures. Actual root-motion dodge clips/ABP remain content integration work.
dodge_data = da.get_editor_property("dodge")
dodge_fields = [("F", "forward_montage"), ("B", "backward_montage"),
                ("L", "left_montage"), ("R", "right_montage")]
for suffix, field in dodge_fields:
    name = f"AM_Warlord_Dodge_{suffix}"
    path = f"{HERO_DIR}/{name}"
    if not assets.does_asset_exist(path):
        factory = unreal.AnimMontageFactory()
        factory.set_editor_property("target_skeleton", skel)
        factory.set_editor_property("source_animation", anim_seq)
        montage = asset_tools.create_asset(name, HERO_DIR, unreal.AnimMontage, factory)
        assert unreal.HeroCombatLibrary.set_single_segment_montage_duration(montage, 0.6)
        assert unreal.HeroCombatLibrary.add_invulnerable_window_to_montage(montage, 0.10, 0.25)
        assert unreal.HeroCombatLibrary.add_cancel_window_to_montage(montage, 0.4, 0.19,
            [unreal.HeroAction.LIGHT, unreal.HeroAction.HEAVY, unreal.HeroAction.BLOCK_START])
        assets.save_loaded_asset(montage, only_if_is_dirty=False)
    else:
        montage = assets.load_asset(path)
    if not dodge_data.get_editor_property(field):
        dodge_data.set_editor_property(field, montage)
da.set_editor_property("dodge", dodge_data)

# Directional hit/death timing fixtures; presentation clips are replaced by the content pass.
react_data = da.get_editor_property("hit_react")
for suffix, field in [("HitReact_F", "front_montage"), ("HitReact_B", "back_montage"), ("Death", "death_montage")]:
    name = f"AM_Warlord_{suffix}"
    path = f"{HERO_DIR}/{name}"
    if not assets.does_asset_exist(path):
        factory = unreal.AnimMontageFactory()
        factory.set_editor_property("target_skeleton", skel)
        factory.set_editor_property("source_animation", anim_seq)
        montage = asset_tools.create_asset(name, HERO_DIR, unreal.AnimMontage, factory)
        assert unreal.HeroCombatLibrary.set_single_segment_montage_duration(montage, 0.6)
        if suffix != "Death":
            assert unreal.HeroCombatLibrary.add_cancel_window_to_montage(montage, 0.4, 0.19, [unreal.HeroAction.DODGE])
        assets.save_loaded_asset(montage, only_if_is_dirty=False)
    else:
        montage = assets.load_asset(path)
    if not react_data.get_editor_property(field):
        react_data.set_editor_property(field, montage)
da.set_editor_property("hit_react", react_data)

# Fill missing references without resetting attack numbers, states or resistance tuning.
attack_data_list = da.get_editor_property("light_chain")
if len(attack_data_list) != 3:
    raise RuntimeError("Existing LightChain must have three entries; refusing to replace custom data")
for i, attack in enumerate(attack_data_list):
    if not attack.get_editor_property("montage"):
        attack.set_editor_property("montage", light_montages[i])
da.set_editor_property("light_chain", attack_data_list)

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
