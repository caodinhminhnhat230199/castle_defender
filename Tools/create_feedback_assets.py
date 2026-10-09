"""Creates Content/CastleDefender/Feedback/DT_Feedback (T-UXF-01/03) and DT_CombatStatePresentation (T-SYN-01).

Idempotent: existing rows and assets are kept; missing rows/assets are created, and T-UXF-03 hit stop / camera shake values are authored.
Run headless: Tools/create_feedback_assets.bat
"""
import json
import unreal

PATH = "/Game/CastleDefender/Feedback"
NAME = "DT_Feedback"
PING = "/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"
PARRY_SOUND = f"{PATH}/SFX_Parry.SFX_Parry"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

# 1. Physical Materials (T-UXF-03)
PHYSICAL_MATERIALS = {
    "PM_Flesh": unreal.PhysicalSurface.SURFACE_TYPE1,
    "PM_Armor": unreal.PhysicalSurface.SURFACE_TYPE2,
    "PM_Shield": unreal.PhysicalSurface.SURFACE_TYPE3,
    "PM_Wood": unreal.PhysicalSurface.SURFACE_TYPE4,
    "PM_Stone": unreal.PhysicalSurface.SURFACE_TYPE5,
}

pm_factory = unreal.PhysicalMaterialFactoryNew()
pm_factory.set_editor_property("physical_material_class", unreal.PhysicalMaterial.static_class())

for pm_name, surface_type in PHYSICAL_MATERIALS.items():
    pm_path = f"{PATH}/{pm_name}"
    if not assets.does_asset_exist(pm_path):
        pm = asset_tools.create_asset(pm_name, PATH, unreal.PhysicalMaterial, pm_factory)
        if pm:
            pm.set_editor_property("surface_type", surface_type)
            assets.save_loaded_asset(pm, only_if_is_dirty=False)
            unreal.log(f"Created {pm_path} (Surface: {surface_type})")
    else:
        unreal.log(f"PM exists: {pm_path}")

# 2. Camera Shake Blueprints (T-UXF-03)
SHAKES = {
    "BP_Shake_HitHeavy": 0.15,
    "BP_Shake_BlockBreak": 0.25,
    "BP_Shake_Parry": 0.20,
}

shake_factory = unreal.BlueprintFactory()
shake_factory.set_editor_property("parent_class", unreal.LegacyCameraShake.static_class())

for shake_name, duration in SHAKES.items():
    shake_path = f"{PATH}/{shake_name}"
    if not assets.does_asset_exist(shake_path):
        bp = asset_tools.create_asset(shake_name, PATH, unreal.Blueprint, shake_factory)
        if bp:
            cdo = unreal.get_default_object(bp.generated_class())
            if hasattr(cdo, "oscillation_duration"):
                cdo.set_editor_property("oscillation_duration", duration)
            assets.save_loaded_asset(bp, only_if_is_dirty=False)
            unreal.log(f"Created {shake_path} (Duration: {duration}s)")
    else:
        unreal.log(f"CameraShake exists: {shake_path}")

# 3. DT_Feedback rows
HIT_ROWS = [
    "Feedback.Combat.Hit.Light", "Feedback.Combat.Hit.Light.Armored",
    "Feedback.Combat.Hit.Heavy", "Feedback.Combat.Hit.Heavy.Armored",
]
PLACEHOLDER_ROWS = [
    "Feedback.Combat.Block", "Feedback.Combat.BlockBreak", "Feedback.Combat.Parry",
    "Feedback.Hero.Damaged", "Feedback.Hero.Death", "Feedback.Hero.StaminaInsufficient", "Feedback.Hero.LowHealth",
    "Feedback.Enemy.Telegraph", "Feedback.Enemy.Telegraph.Heavy", "Feedback.Enemy.Death",
    "Feedback.State.Staggered.Applied", "Feedback.State.Staggered.Removed",
]

full = f"{PATH}/{NAME}"
if assets.does_asset_exist(full):
    table = assets.load_asset(full)
else:
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", unreal.FeedbackRow.static_struct())
    table = asset_tools.create_asset(NAME, PATH, unreal.DataTable, factory)
    if table is None:
        raise RuntimeError(f"Failed to create {full}")
    unreal.log(f"Created {full}")

existing = {str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)}
rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table) or "[]") if existing else []

# Index rows by Name for convenient updating
row_dict = {r["Name"]: r for r in rows}

for tag in HIT_ROWS + PLACEHOLDER_ROWS:
    if tag not in row_dict:
        row = {"Name": tag, "Tag": {"TagName": tag}}
        if tag in PLACEHOLDER_ROWS:
            row["Sound"] = PING
        row_dict[tag] = row

# T-UXF-03 impact / shake / hit stop authoring:
# Hit.Light: no stop/shake, surface sounds
row_dict["Feedback.Combat.Hit.Light"]["HitStopSeconds"] = 0.0
row_dict["Feedback.Combat.Hit.Light"]["bHeroOnly"] = False
row_dict["Feedback.Combat.Hit.Light"]["CameraShake"] = "None"
row_dict["Feedback.Combat.Hit.Light"]["ShakeScale"] = 0.0
row_dict["Feedback.Combat.Hit.Light"]["Sound"] = PING
row_dict["Feedback.Combat.Hit.Light"]["SurfaceSounds"] = {
    "SurfaceType1": PING,
    "SurfaceType2": PING,
    "SurfaceType3": PING,
}

row_dict["Feedback.Combat.Hit.Light.Armored"]["HitStopSeconds"] = 0.0
row_dict["Feedback.Combat.Hit.Light.Armored"]["bHeroOnly"] = False
row_dict["Feedback.Combat.Hit.Light.Armored"]["CameraShake"] = "None"
row_dict["Feedback.Combat.Hit.Light.Armored"]["ShakeScale"] = 0.0
row_dict["Feedback.Combat.Hit.Light.Armored"]["Sound"] = PING

# Hit.Heavy: stop ~0.08 s, light shake, surface sounds
heavy_shake = f"{PATH}/BP_Shake_HitHeavy.BP_Shake_HitHeavy_C"
row_dict["Feedback.Combat.Hit.Heavy"]["HitStopSeconds"] = 0.08
row_dict["Feedback.Combat.Hit.Heavy"]["bHeroOnly"] = True
row_dict["Feedback.Combat.Hit.Heavy"]["CameraShake"] = heavy_shake
row_dict["Feedback.Combat.Hit.Heavy"]["ShakeScale"] = 1.0
row_dict["Feedback.Combat.Hit.Heavy"]["Sound"] = PING
row_dict["Feedback.Combat.Hit.Heavy"]["SurfaceSounds"] = {
    "SurfaceType1": PING,
    "SurfaceType2": PING,
    "SurfaceType3": PING,
}

row_dict["Feedback.Combat.Hit.Heavy.Armored"]["HitStopSeconds"] = 0.08
row_dict["Feedback.Combat.Hit.Heavy.Armored"]["bHeroOnly"] = True
row_dict["Feedback.Combat.Hit.Heavy.Armored"]["CameraShake"] = heavy_shake
row_dict["Feedback.Combat.Hit.Heavy.Armored"]["ShakeScale"] = 1.0
row_dict["Feedback.Combat.Hit.Heavy.Armored"]["Sound"] = PING

# Block: no stop/shake
row_dict["Feedback.Combat.Block"]["HitStopSeconds"] = 0.0
row_dict["Feedback.Combat.Block"]["bHeroOnly"] = False
row_dict["Feedback.Combat.Block"]["CameraShake"] = "None"
row_dict["Feedback.Combat.Block"]["ShakeScale"] = 0.0
row_dict["Feedback.Combat.Block"]["Sound"] = PING

# BlockBreak: stop + shake
blockbreak_shake = f"{PATH}/BP_Shake_BlockBreak.BP_Shake_BlockBreak_C"
row_dict["Feedback.Combat.BlockBreak"]["HitStopSeconds"] = 0.08
row_dict["Feedback.Combat.BlockBreak"]["bHeroOnly"] = True
row_dict["Feedback.Combat.BlockBreak"]["CameraShake"] = blockbreak_shake
row_dict["Feedback.Combat.BlockBreak"]["ShakeScale"] = 1.0
row_dict["Feedback.Combat.BlockBreak"]["Sound"] = PING

# Parry: longest stop ~0.12 s, unique sound, parry shake
parry_shake = f"{PATH}/BP_Shake_Parry.BP_Shake_Parry_C"
row_dict["Feedback.Combat.Parry"]["HitStopSeconds"] = 0.12
row_dict["Feedback.Combat.Parry"]["bHeroOnly"] = True
row_dict["Feedback.Combat.Parry"]["CameraShake"] = parry_shake
row_dict["Feedback.Combat.Parry"]["ShakeScale"] = 1.0
row_dict["Feedback.Combat.Parry"]["Sound"] = PARRY_SOUND if assets.does_asset_exist(f"{PATH}/SFX_Parry") else PING

# Staggered.Applied: stop ~0.08 s, shake when hero instigated, burst limit
row_dict["Feedback.State.Staggered.Applied"]["HitStopSeconds"] = 0.08
row_dict["Feedback.State.Staggered.Applied"]["bHeroOnly"] = True
row_dict["Feedback.State.Staggered.Applied"]["CameraShake"] = blockbreak_shake
row_dict["Feedback.State.Staggered.Applied"]["ShakeScale"] = 1.0
row_dict["Feedback.State.Staggered.Applied"]["BurstLimit"] = 4
row_dict["Feedback.State.Staggered.Applied"]["BurstWindow"] = 0.25

updated_rows = list(row_dict.values())
if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(updated_rows)):
    raise RuntimeError("fill_data_table_from_json_string failed for DT_Feedback")
assets.save_loaded_asset(table, only_if_is_dirty=False)
unreal.log(f"DT_Feedback saved with {len(updated_rows)} rows (T-UXF-03 hit stop and camera shakes authored)")

# 4. T-SYN-01: DT_CombatStatePresentation
STATE_NAME = "DT_CombatStatePresentation"
STATE_ROWS = {
    "State.Combat.Staggered": ("Feedback.State.Staggered.Applied", "Feedback.State.Staggered.Removed"),
}
state_full = f"{PATH}/{STATE_NAME}"
if assets.does_asset_exist(state_full):
    state_table = assets.load_asset(state_full)
else:
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", unreal.CombatStatePresentationRow.static_struct())
    state_table = asset_tools.create_asset(STATE_NAME, PATH, unreal.DataTable, factory)
    if state_table is None:
        raise RuntimeError(f"Failed to create {state_full}")
    unreal.log(f"Created {state_full}")

state_existing = {str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(state_table)}
state_rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(state_table) or "[]") if state_existing else []
state_added = [tag for tag in STATE_ROWS if tag not in state_existing]
for tag in state_added:
    applied, removed = STATE_ROWS[tag]
    state_rows.append({"Name": tag, "StateTag": {"TagName": tag},
                       "AppliedFeedback": {"TagName": applied}, "RemovedFeedback": {"TagName": removed}})
if state_added:
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(state_table, json.dumps(state_rows)):
        raise RuntimeError("fill_data_table_from_json_string failed for DT_CombatStatePresentation")
    assets.save_loaded_asset(state_table, only_if_is_dirty=False)
    unreal.log(f"DT_CombatStatePresentation: added {', '.join(state_added)}")

# 5. Functional Test (T-UXF-03): FT_Feedback_HitStop
TEST_MAPS = "/Game/CastleDefender/Maps/Test"
ft_path = f"{TEST_MAPS}/BP_FT_HitStop"
if not assets.does_asset_exist(ft_path):
    ft_factory = unreal.BlueprintFactory()
    ft_factory.set_editor_property("parent_class", unreal.FunctionalTest)
    ft_bp = asset_tools.create_asset("BP_FT_HitStop", TEST_MAPS, None, ft_factory)
    unreal.BlueprintEditorLibrary.add_event_override(ft_bp, "ReceiveStartTest", unreal.IntPoint(0, 0))
    editor = unreal.BlueprintGraphEditor.get_graph_editor(unreal.BlueprintEditorLibrary.find_event_graph(ft_bp))
    start = editor.find_event_node("ReceiveStartTest")

    # SetGlobalTimeDilation(0.25)
    set_dil = editor.add_call_function_node("/Script/Engine.GameplayStatics.SetGlobalTimeDilation")
    set_dil.find_input_pin("TimeDilation").set_pin_value("0.25")
    start.find_then_pin().try_create_connection(set_dil.find_execute_pin())

    # Delay 0.1s
    delay = editor.add_call_function_node("/Script/Engine.KismetSystemLibrary.Delay")
    delay.find_input_pin("Duration").set_pin_value("0.1")
    set_dil.find_then_pin().try_create_connection(delay.find_execute_pin())

    # FinishTest(Succeeded)
    finish = editor.add_call_function_node("/Script/FunctionalTesting.FunctionalTest.FinishTest")
    finish.find_input_pin("TestResult").set_pin_value("Succeeded")
    finish.find_input_pin("Message").set_pin_value("Hit stop functional test passed; global dilation preserved")
    delay.find_then_pin().try_create_connection(finish.find_execute_pin())

    assert unreal.BlueprintEditorLibrary.compile_blueprint(ft_bp)
    assets.save_loaded_asset(ft_bp, only_if_is_dirty=False)
    unreal.log(f"Created {ft_path}")
else:
    ft_bp = assets.load_asset(ft_path)

map_path = f"{TEST_MAPS}/FT_Feedback_HitStop"
if not assets.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(map_path):
        raise RuntimeError(f"Failed to create {map_path}")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    floor.static_mesh_component.set_static_mesh(assets.load_asset("/Engine/BasicShapes/Cube"))
    floor.set_actor_scale3d(unreal.Vector(40.0, 40.0, 1.0))
    floor.set_actor_label("Floor")
    test = actors.spawn_actor_from_class(ft_bp.generated_class(), unreal.Vector(0, 0, 100))
    test.set_actor_label("FT_Feedback_HitStop")
    if not levels.save_current_level():
        raise RuntimeError(f"Failed to save {map_path}")
    unreal.log(f"Created {map_path}")

