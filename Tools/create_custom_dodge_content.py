"""T-CMB-22 saved custom dodge/afterimage content. Run in Unreal Editor after build.

Creates missing original assets and preserves existing authored assets on subsequent runs.
Use CUSTOM_DODGE_REBAKE=1 explicitly to rebake our animation sources while iterating.
Existing packages are backed up before any save; no source asset is removed.
"""
import json
import os
import shutil
from pathlib import Path
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
root = Path(unreal.Paths.project_dir()).resolve()
backup = root / "Saved/Backups/custom-dodge-content"

def save(asset):
    relative = asset.get_path_name().split('.')[0].removeprefix('/Game/') + '.uasset'
    source, target = root / 'Content' / relative, backup / relative
    if source.exists() and not target.exists():
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    assert assets.save_loaded_asset(asset, only_if_is_dirty=False), asset.get_path_name()

idle = assets.load_asset('/Game/CastleDefender/Placeholder/Mannequins/Anims/Unarmed/MM_Idle')
hero = assets.load_asset('/Game/CastleDefender/Hero/DA_HeroClass_Warlord')
assert idle and hero, 'Saved mannequin idle and Warlord are prerequisites'
dodge = hero.get_editor_property('dodge')
result = []
for short, direction, field in [
    ('F', unreal.HeroDodgeDirection.FORWARD, 'forward_montage'),
    ('B', unreal.HeroDodgeDirection.BACKWARD, 'backward_montage'),
    ('L', unreal.HeroDodgeDirection.LEFT, 'left_montage'),
    ('R', unreal.HeroDodgeDirection.RIGHT, 'right_montage'),
]:
    path = '/Game/CastleDefender/Hero/A_Warlord_CustomDodge_' + short
    new = not assets.does_asset_exist(path)
    if new:
        factory = unreal.AnimSequenceFactory()
        factory.set_editor_property('target_skeleton', idle.get_editor_property('skeleton'))
        clip = unreal.AssetToolsHelpers.get_asset_tools().create_asset(path.rsplit('/', 1)[1], '/Game/CastleDefender/Hero', unreal.AnimSequence, factory)
    else:
        clip = assets.load_asset(path)
    assert clip, path
    if new or os.environ.get('CUSTOM_DODGE_REBAKE') == '1':
        assert unreal.HeroCombatLibrary.author_custom_dodge(clip, idle, direction, 0.6, 500.0), path
        save(clip)
    montage = dodge.get_editor_property(field)
    assert montage and abs(montage.get_play_length() - 0.6) < 1e-4
    current = montage.get_editor_property('slot_anim_tracks')[0].get_editor_property('anim_track').get_editor_property('anim_segments')[0].get_editor_property('anim_reference')
    if current != clip:
        assert unreal.HeroCombatLibrary.set_single_segment_montage_source(montage, clip, 0.0, clip.get_play_length(), 0.6, False)
        save(montage)
    result.append({'direction': short, 'clip': clip.get_path_name(), 'montage': montage.get_path_name(), 'duration': clip.get_play_length()})

if not dodge.get_editor_property('enable_perfect_dodge'):
    dodge.set_editor_property('enable_perfect_dodge', True)
    dodge.set_editor_property('perfect_window_seconds', 0.12)
    hero.set_editor_property('dodge', dodge)
    save(hero)

material_path = '/Game/CastleDefender/Feedback/M_PerfectDodgeGhost'
if not assets.does_asset_exist(material_path):
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_PerfectDodgeGhost', '/Game/CastleDefender/Feedback', unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('two_sided', False)
    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 0)
    color.set_editor_property('parameter_name', 'GhostColor')
    color.set_editor_property('default_value', unreal.LinearColor(0.20, 0.75, 1.0, 1.0))
    opacity = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -400, 200)
    opacity.set_editor_property('parameter_name', 'GhostOpacity')
    opacity.set_editor_property('default_value', 0.55)
    assert unreal.MaterialEditingLibrary.connect_material_property(color, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    assert unreal.MaterialEditingLibrary.connect_material_property(opacity, '', unreal.MaterialProperty.MP_OPACITY)
    unreal.MaterialEditingLibrary.recompile_material(material)
    save(material)

# Runtime frozen poses need the skeletal usage permutation serialized into the material.
material = assets.load_asset(material_path)
if not material.get_editor_property('used_with_skeletal_mesh'):
    unreal.MaterialEditingLibrary.set_base_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH, True)
    unreal.MaterialEditingLibrary.recompile_material(material)
    save(material)

sound_path = '/Game/CastleDefender/Feedback/SFX_PerfectDodge'
if not assets.does_asset_exist(sound_path):
    source = unreal.load_object(None, '/Engine/EditorSounds/GamePreview/StartPlayInEditor.StartPlayInEditor')
    assert source, 'Engine sound prerequisite'
    sound = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset('SFX_PerfectDodge', '/Game/CastleDefender/Feedback', source)
    assert sound
    save(sound)

table = assets.load_asset('/Game/CastleDefender/Feedback/DT_Feedback')
rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table))
tag = 'Feedback.Combat.PerfectDodge'
if not any(row['Name'] == tag for row in rows):
    rows.append({'Name': tag, 'Tag': {'TagName': tag}, 'Sound': sound_path + '.SFX_PerfectDodge',
                 'AfterimageMaterial': material_path + '.M_PerfectDodgeGhost',
                 'AfterimageSeconds': 0.35, 'AfterimageOpacity': 0.55})
    assert unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json.dumps(rows))
    save(table)
Path(unreal.Paths.project_saved_dir(), 'custom-dodge-content.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('CUSTOM_DODGE_CONTENT ' + json.dumps(result))
