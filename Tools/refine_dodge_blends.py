"""Requested quick-step presentation refinement; preserves gameplay windows and source clips.

Run in Unreal Editor. Only the stock 0.25-second dodge blends are refined; other tuning is kept.
Existing packages are backed up before saving. This does not author roll or perfect-dodge animations.
"""
import json
import shutil
from pathlib import Path
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
root = Path(unreal.Paths.project_dir()).resolve()
backup = root / "Saved/Backups/dodge-presentation-blends"
definition = assets.load_asset("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")
assert definition, "Missing saved hero definition"
dodge = definition.get_editor_property("dodge")
result = []
for field in ["forward_montage", "backward_montage", "left_montage", "right_montage"]:
    montage = dodge.get_editor_property(field)
    assert montage, f"Missing {field}"
    row = {"path": montage.get_path_name(), "duration": montage.get_play_length()}
    changed = False
    for prop, desired in [("blend_in", 0.06), ("blend_out", 0.10)]:
        blend = montage.get_editor_property(prop)
        previous = blend.get_editor_property("blend_time")
        row[prop] = {"before": previous, "after": previous}
        if abs(previous - 0.25) < 1e-5:
            relative = montage.get_path_name().split(".")[0].removeprefix("/Game/") + ".uasset"
            source, target = root / "Content" / relative, backup / relative
            assert source.is_file(), f"Missing package {source}"
            if not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
            blend.set_editor_property("blend_time", desired)
            montage.set_editor_property(prop, blend)
            row[prop]["after"] = desired
            changed = True
    if changed:
        assert assets.save_loaded_asset(montage, only_if_is_dirty=False), f"Save failed: {montage}"
    result.append(row)
Path(unreal.Paths.project_saved_dir(), "dodge-blend-refinement.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
unreal.log("DODGE_BLEND_REFINEMENT " + json.dumps(result))
