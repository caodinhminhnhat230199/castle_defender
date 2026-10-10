"""T-SYN-02: additive Heavy/state authoring, dedicated placeholder sound and armored test definition.

Run in Unreal Editor after building. Existing tuning, rows, graphs and montages are preserved.
Existing packages are backed up under Saved/Backups before changing them.
"""
import json
import shutil
from pathlib import Path
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
root = Path(unreal.Paths.project_dir()).resolve()
backup = root / "Saved/Backups/armor-break-content"


def save(asset):
    relative = asset.get_path_name().split(".")[0].removeprefix("/Game/") + ".uasset"
    source = root / "Content" / relative
    target = backup / relative
    if source.exists() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    if not assets.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {asset.get_path_name()}")


def add_rows(path, additions):
    table = assets.load_asset(path)
    assert table, f"Missing prerequisite table {path}"
    rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table) or "[]")
    names = {row["Name"] for row in rows}
    missing = [row for row in additions if row["Name"] not in names]
    if missing:
        if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(rows + missing)):
            raise RuntimeError(f"Failed to add rows to {path}")
        save(table)


tag = unreal.GameplayTag()
tag.import_text('(TagName="State.Combat.ArmorBroken")')
hero = assets.load_asset("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")
assert hero, "T-CMB-06 saved hero definition required"
heavy = hero.get_editor_property("heavy")
states = heavy.get_editor_property("applied_states")
if not unreal.GameplayTagLibrary.has_tag(states, tag, True):
    states = unreal.GameplayTagLibrary.add_gameplay_tag(states, tag)
    heavy.set_editor_property("applied_states", states)
    hero.set_editor_property("heavy", heavy)
    save(hero)

sound_path = "/Game/CastleDefender/Feedback/SFX_ArmorBreak"
if not assets.does_asset_exist(sound_path):
    source_sound = unreal.load_object(None, "/Engine/EditorSounds/GamePreview/StartSimulate.StartSimulate")
    assert source_sound, "Missing engine sound source"
    sound = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset("SFX_ArmorBreak", "/Game/CastleDefender/Feedback", source_sound)
    assert sound, "Could not create the dedicated licensed armor-break placeholder"
    save(sound)

applied = "Feedback.State.ArmorBroken.Applied"
add_rows("/Game/CastleDefender/Feedback/DT_Feedback", [
    {"Name": applied, "Tag": {"TagName": applied}, "Sound": sound_path + ".SFX_ArmorBreak"},
])
add_rows("/Game/CastleDefender/Feedback/DT_CombatStatePresentation", [
    {"Name": "State.Combat.ArmorBroken", "StateTag": {"TagName": "State.Combat.ArmorBroken"},
     "AppliedFeedback": {"TagName": applied}},
])

# Fixture only, not T-ENM-06's Armored archetype: reuse existing body/AI/attacks.
enemy_path = "/Game/CastleDefender/Enemy/DA_Enemy_ArmorTest"
if not assets.does_asset_exist(enemy_path):
    enemy = assets.duplicate_asset("/Game/CastleDefender/Enemy/DA_Enemy_Test", enemy_path)
    assert enemy, "T-ENM-01 saved test definition required"
    enemy.set_editor_property("display_name", unreal.Text("Armor Test Enemy"))
    enemy.set_editor_property("base_armor", 0.5)
    enemy.set_editor_property("max_health", 1000.0)
    save(enemy)

unreal.log("ARMOR_BREAK_CONTENT: Heavy, state/feedback rows, sound and test enemy ready; existing entries preserved")
