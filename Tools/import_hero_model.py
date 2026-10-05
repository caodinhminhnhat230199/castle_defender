"""Imports the single FBX in SourceModels/Hero/ (Tripo3D, UE5 Mannequin rig) as the Warlord mesh.

The mesh goes onto the existing Placeholder SK_Mannequin so ABP_Warlord and every AM_Warlord_* montage keep working.
It is scaled to SKM_Manny_Simple's height, checked bone by bone against Manny's hierarchy, and only then assigned
to BP_Hero_Warlord. Each run reimports from the FBX, so a re-exported file just needs a rerun.
Run headless: Tools/import_hero_model.bat (editor closed).
"""
from pathlib import Path

import unreal

PROJECT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCES = sorted((PROJECT / "SourceModels" / "Hero").glob("*.fbx"))
if len(SOURCES) != 1:
    raise RuntimeError(f"Expected exactly one FBX in SourceModels/Hero, found {[s.name for s in SOURCES]}")
FBX = SOURCES[0]
DEST = "/Game/CastleDefender/Placeholder/Characters/Warlord"
MESH = f"{DEST}/SKM_Warlord"
SKELETON = "/Game/CastleDefender/Placeholder/Mannequins/Meshes/SK_Mannequin"
MANNY = "/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple"
HERO_BP = "/Game/CastleDefender/Hero/BP_Hero_Warlord"
REQUIRED_BONES = ["root", "pelvis", "spine_01", "head", "hand_r", "hand_l", "foot_r", "foot_l"]
HEIGHT_TOLERANCE = 0.03

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()


def import_fbx(scale):
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("skeleton", assets.load_asset(SKELETON))
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("create_physics_asset", False)
    ui.set_editor_property("import_materials", True)
    ui.set_editor_property("import_textures", True)
    ui.get_editor_property("skeletal_mesh_import_data").set_editor_property("import_uniform_scale", scale)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(FBX))
    task.set_editor_property("destination_path", DEST)
    task.set_editor_property("destination_name", "SKM_Warlord")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", ui)
    tools.import_asset_tasks([task])
    unreal.log(f"Imported at scale {scale:.4f}: {[str(p) for p in task.get_editor_property('imported_object_paths')]}")
    mesh = assets.load_asset(MESH)
    if not mesh:
        raise RuntimeError(f"{MESH} was not created; see the FBX import log above")
    return mesh


def hierarchy(skeletal_mesh):
    comp = unreal.new_object(unreal.SkeletalMeshComponent)
    comp.set_skeletal_mesh_asset(skeletal_mesh)
    return {str(comp.get_bone_name(i)): str(comp.get_parent_bone(comp.get_bone_name(i))) for i in range(comp.get_num_bones())}


def height(skeletal_mesh):
    return skeletal_mesh.get_bounds().box_extent.z * 2.0


if not FBX.exists():
    raise RuntimeError(f"Missing {FBX}")
manny = assets.load_asset(MANNY)
manny_tree = hierarchy(manny)
target_height = height(manny)

mesh = import_fbx(1.0)
if abs(height(mesh) - target_height) > target_height * HEIGHT_TOLERANCE:
    mesh = import_fbx(target_height / height(mesh))

# Every Manny bone keeps Manny's parent, otherwise the mesh cannot share SK_Mannequin (UE4 rigs stop here).
# Extra bones (e.g. Reallusion cc_base_* face/twist bones) are allowed: the import adds them to the skeleton,
# and Manny animations leave them at their reference pose.
tree = hierarchy(mesh)
skeleton = assets.load_asset(SKELETON)
skeleton_bones = {str(b) for b in unreal.AnimPoseExtensions.get_bone_names(unreal.AnimPoseExtensions.get_reference_pose(skeleton))}
missing = [b for b in REQUIRED_BONES if b not in tree]
extra = [b for b in tree if b not in manny_tree]
not_merged = [b for b in tree if b not in skeleton_bones]
reparented = [f"{b} (parent {p}, Manny {manny_tree[b]})" for b, p in tree.items() if b in manny_tree and manny_tree[b] != p]
unreal.log(f"SKM_Warlord: {len(tree)} bones (Manny {len(manny_tree)}, {len(extra)} extra), height {height(mesh):.1f} cm "
           f"(Manny {target_height:.1f} cm)")
if missing or reparented or not_merged or mesh.get_editor_property("skeleton") != skeleton:
    raise RuntimeError(f"SKM_Warlord does not match the UE5 Mannequin: missing {missing}, reparented {reparented}, "
                       f"not merged into SK_Mannequin {not_merged}. Not assigned to BP_Hero_Warlord.")
if extra:
    # The import merged the extra bones into SK_Mannequin in memory; save it so the mesh stays compatible on load.
    assets.save_loaded_asset(skeleton, only_if_is_dirty=False)
    unreal.log(f"SK_Mannequin saved with {len(extra)} extra bones from SKM_Warlord")
if abs(height(mesh) - target_height) > target_height * HEIGHT_TOLERANCE:
    raise RuntimeError(f"Scaled height {height(mesh):.1f} cm is not within {HEIGHT_TOLERANCE:.0%} of Manny")

# Base colour: the rigged FBX carries no texture, so the Tripo Texture.jpg drives a minimal M_Warlord.
TEXTURE = FBX.with_name("Texture.jpg")
material = assets.load_asset(f"{DEST}/M_Warlord") if assets.does_asset_exist(f"{DEST}/M_Warlord") else None
if TEXTURE.exists() and not material:
    tex_task = unreal.AssetImportTask()
    tex_task.set_editor_property("filename", str(TEXTURE))
    tex_task.set_editor_property("destination_path", DEST)
    tex_task.set_editor_property("destination_name", "T_Warlord_BaseColor")
    tex_task.set_editor_property("automated", True)
    tex_task.set_editor_property("replace_existing", True)
    tex_task.set_editor_property("save", True)
    tools.import_asset_tasks([tex_task])
    texture = assets.load_asset(f"{DEST}/T_Warlord_BaseColor")
    material = tools.create_asset("M_Warlord", DEST, unreal.Material, unreal.MaterialFactoryNew())
    mel = unreal.MaterialEditingLibrary
    sample = mel.create_material_expression(material, unreal.MaterialExpressionTextureSample, -400, 0)
    sample.set_editor_property("texture", texture)
    mel.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = mel.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 250)
    roughness.set_editor_property("r", 0.8)
    mel.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(material)
    assets.save_loaded_asset(material, only_if_is_dirty=False)
    unreal.log("Created M_Warlord from Texture.jpg")
if material and not material.get_editor_property("used_with_skeletal_mesh"):
    # Without the usage flag a cooked build renders the default material on skinned meshes.
    material.set_editor_property("used_with_skeletal_mesh", True)
    unreal.MaterialEditingLibrary.recompile_material(material)
    assets.save_loaded_asset(material, only_if_is_dirty=False)
    unreal.log("M_Warlord: Used with Skeletal Mesh enabled")
# Swap the visible mesh only; ABP_Warlord, the capsule offset and the -90 yaw stay as authored for Manny.
# M_Warlord goes on the component as an override: the FBX import resets the mesh's own slot on every reimport.
hero_bp = assets.load_asset(HERO_BP)
hero_mesh = unreal.get_default_object(hero_bp.generated_class()).get_editor_property("mesh")
changed = hero_mesh.get_skeletal_mesh_asset() != mesh
if changed:
    hero_mesh.set_skeletal_mesh_asset(mesh)
if material:
    overrides = [material] * len(mesh.get_editor_property("materials"))
    if list(hero_mesh.get_editor_property("override_materials")) != overrides:
        hero_mesh.set_editor_property("override_materials", overrides)
        changed = True
if changed:
    unreal.BlueprintEditorLibrary.compile_blueprint(hero_bp)
    assets.save_loaded_asset(hero_bp, only_if_is_dirty=False)
    unreal.log(f"BP_Hero_Warlord uses SKM_Warlord with {'M_Warlord' if material else 'its imported material'}")
else:
    unreal.log("BP_Hero_Warlord already up to date")
