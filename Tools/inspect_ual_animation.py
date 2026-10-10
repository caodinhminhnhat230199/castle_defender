"""Read-only imported animation / saved hero inventory, Unreal Editor Python."""
import json
from pathlib import Path
import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
result = {'animations': [], 'meshes': [], 'graphs': [], 'api': {}}
for path in assets.list_assets('/Game/CastleDefender/Assets', recursive=True):
    data = assets.find_asset_data(path)
    kind = str(data.asset_class_path.asset_name)
    if kind not in ('AnimSequence', 'SkeletalMesh'):
        continue
    obj = data.get_asset()
    skel = obj.get_editor_property('skeleton')
    row = {'path': path, 'skeleton': skel.get_path_name()}
    if kind == 'SkeletalMesh':
        comp = unreal.new_object(unreal.SkeletalMeshComponent)
        comp.set_skeletal_mesh_asset(obj)
        row['bones'] = {str(comp.get_bone_name(i)): str(comp.get_parent_bone(comp.get_bone_name(i))) for i in range(comp.get_num_bones())}
        row['height'] = obj.get_bounds().box_extent.z * 2
        result['meshes'].append(row)
    else:
        row.update(length=obj.get_play_length(), frames=unreal.AnimationLibrary.get_num_frames(obj), root_motion=obj.get_editor_property('enable_root_motion'))
        if any(word in path for word in ('Shield_Dash_RM', 'Sword_Dash_RM', 'Sword_Regular_', 'Hit_Knockback', 'Jump_', 'Sword_Idle')):
            row['samples'] = []
            options = unreal.AnimPoseEvaluationOptions()
            options.set_editor_property('extract_root_motion', False)
            for i in range(9):
                pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(obj, obj.get_play_length() * i / 8, options)
                row['samples'].append({str(b): unreal.AnimPoseExtensions.get_bone_pose(pose, b, unreal.AnimPoseSpaces.WORLD).export_text() for b in ['root', 'pelvis', 'hand_r', 'head']})
        result['animations'].append(row)
hero = assets.load_asset('/Game/CastleDefender/Hero/BP_Hero_Warlord')
cdo = unreal.get_default_object(hero.generated_class())
mesh = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
result['hero_mesh'] = mesh.get_editor_property('skeletal_mesh_asset').get_path_name()
result['hero_skeleton'] = mesh.get_editor_property('skeletal_mesh_asset').get_editor_property('skeleton').get_path_name()
abp = assets.load_asset('/Game/CastleDefender/Hero/ABP_Warlord')
for graph in unreal.BlueprintEditorLibrary.list_graphs(abp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    nodes = []
    for node in editor.list_all_nodes():
        row = {'name': node.get_name(), 'class': node.get_class().get_name()}
        try:
            inner = node.get_editor_property('node')
            row['data'] = inner.export_text()
        except Exception:
            pass
        nodes.append(row)
    result['graphs'].append({'name': graph.get_name(), 'nodes': nodes})
for name in ['IKRigController', 'IKRetargeterController', 'IKRetargetBatchOperation', 'AnimationLibrary', 'AnimPoseExtensions', 'BlueprintEditorLibrary']:
    cls = getattr(unreal, name, None)
    result['api'][name] = {n: getattr(cls, n).__doc__ for n in dir(cls) if any(key in n for key in ('auto_', 'retarget', 'pose_at', 'bone_pose', 'graph', 'root', 'chain'))} if cls else None
Path(unreal.Paths.project_saved_dir(), 'ual-animation-inventory.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
unreal.log('UAL_INVENTORY_COMPLETE')
