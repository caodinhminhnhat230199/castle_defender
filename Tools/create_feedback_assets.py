"""Creates Content/CastleDefender/Feedback/DT_Feedback (T-UXF-01) and DT_CombatStatePresentation (T-SYN-01).

Idempotent: existing rows and their edits are kept; only missing rows are added.
Placeholder: non-hit rows play the engine 1 kHz ping until owners author real assets;
hit rows stay empty for T-UXF-03.
Run headless: Tools/create_feedback_assets.bat
"""
import json

import unreal

PATH = "/Game/CastleDefender/Feedback"
NAME = "DT_Feedback"
PING = "/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"

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

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
full = f"{PATH}/{NAME}"

if assets.does_asset_exist(full):
    table = assets.load_asset(full)
else:
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", unreal.FeedbackRow.static_struct())
    table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, unreal.DataTable, factory)
    if table is None:
        raise RuntimeError(f"Failed to create {full}")
    unreal.log(f"Created {full}")

existing = {str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)}
rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table) or "[]") if existing else []

added = []
for tag in HIT_ROWS + PLACEHOLDER_ROWS:
    if tag in existing:
        continue
    row = {"Name": tag, "Tag": {"TagName": tag}}
    if tag in PLACEHOLDER_ROWS:
        row["Sound"] = PING
    rows.append(row)
    added.append(tag)

if added:
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(rows)):
        raise RuntimeError("fill_data_table_from_json_string failed")
    assets.save_loaded_asset(table, only_if_is_dirty=False)
    unreal.log(f"DT_Feedback: added {len(added)} rows: {', '.join(added)}")
else:
    unreal.log("DT_Feedback: all P0 rows present; nothing changed")

# Readback: every P0 row exists and its tag equals its row name.
names = {str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)}
missing = [t for t in HIT_ROWS + PLACEHOLDER_ROWS if t not in names]
if missing:
    raise RuntimeError(f"DT_Feedback is missing rows: {missing}")
unreal.log(f"DT_Feedback OK: {len(names)} rows")

# T-SYN-01: DT_CombatStatePresentation with the Staggered row (row name = state tag).
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
    state_table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(STATE_NAME, PATH, unreal.DataTable, factory)
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
for row in json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(state_table)):
    unreal.log(f"DT_CombatStatePresentation OK: {row['Name']} -> {row['AppliedFeedback']['TagName']} / {row['RemovedFeedback']['TagName']}")
