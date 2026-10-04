"""Creates Hero assets: DA_HeroClass_Warlord and BP_Hero_Warlord, and configures BP_SandboxGameMode.

Idempotent: can be run headless or in editor.
Run headless: Tools/create_hero_assets.bat
"""
import gc
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

if not assets.does_asset_exist(da_path):
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", hero_def_class)
    da = asset_tools.create_asset("DA_HeroClass_Warlord", HERO_DIR, hero_def_class, factory)
else:
    da = assets.load_asset(da_path)

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
    assets.save_loaded_asset(gm_bp, only_if_is_dirty=False)
    unreal.log("Updated BP_SandboxGameMode default_pawn_class to BP_Hero_Warlord")

print("Hero assets created and configured successfully.")
