"""T-ENM-11: one playable melee archetype, red Quinn variant and separate 1/3/5 areas.

Seed the new DA from the existing fixture as an explicit initial tuning baseline.
Preserve existing tuning, Blueprints and placed actors; add only missing owned content.
"""
import math
import unreal

ENEMY = "/Game/CastleDefender/Enemy"
PLACEHOLDER = "/Game/CastleDefender/Placeholder/Enemy"
MAP = "/Game/CastleDefender/Maps/L_CombatSandbox"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()
base = assets.load_asset(ENEMY + "/BP_Enemy_Base")
fixture = assets.load_asset(ENEMY + "/DA_Enemy_Test")
assert base and fixture, "Run create_enemy_assets.bat first"

bp_path = ENEMY + "/BP_Enemy_Melee"
if assets.does_asset_exist(bp_path):
    bp = assets.load_asset(bp_path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", base.generated_class())
    bp = tools.create_asset("BP_Enemy_Melee", ENEMY, None, factory)
    defaults = unreal.get_default_object(bp.generated_class())
    mesh = defaults.mesh
    assets.make_directory(PLACEHOLDER)
    for index in range(mesh.get_num_materials()):
        source = mesh.get_material(index)
        name = "MI_Enemy_Melee_Red_" + str(index + 1).zfill(2)
        path = PLACEHOLDER + "/" + name
        material = assets.load_asset(path) if assets.does_asset_exist(path) else assets.duplicate_asset(source.get_path_name(), path)
        assert "Paint Tint" in [str(n) for n in unreal.MaterialEditingLibrary.get_vector_parameter_names(material)]
        # UE 5.8's setter returns false unconditionally; validate its actual parameter value instead.
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, "Paint Tint", unreal.LinearColor(0.55, 0.025, 0.02, 1))
        tint = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(material, "Paint Tint")
        assert abs(tint.r-0.55) < 0.001 and abs(tint.g-0.025) < 0.001 and abs(tint.b-0.02) < 0.001
        assets.save_loaded_asset(material, only_if_is_dirty=False)
        mesh.set_material(index, material)
    assert unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert assets.save_loaded_asset(bp, only_if_is_dirty=False)

da_path = ENEMY + "/DA_Enemy_Melee"
if assets.does_asset_exist(da_path):
    definition = assets.load_asset(da_path)
else:
    definition = assets.duplicate_asset(fixture.get_path_name(), da_path)
    definition.set_editor_property("display_name", unreal.Text("Melee Enemy"))
    definition.set_editor_property("enemy_class", bp.generated_class())
    assert assets.save_loaded_asset(definition, only_if_is_dirty=False)
assert len(definition.get_editor_property("attacks")) == 2

world = unreal.EditorLoadingAndSavingUtils.load_map(MAP)
assert world
existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
added = []
# Areas are over two aggro radii apart and clear of the five preserved fixture enemies on the east side.
for count, center in ((1, (-1300, -650)), (3, (-1300, 1200)), (5, (1300, 1600))):
    for index in range(count):
        label = "Melee_" + str(count) + "_" + str(index + 1)
        if label in existing:
            continue
        angle = 2 * math.pi * index / count
        radius = 0 if count == 1 else 130
        location = unreal.Vector(center[0] + radius*math.cos(angle), center[1] + radius*math.sin(angle), 88)
        actor = actors.spawn_actor_from_class(bp.generated_class(), location)
        assert actor
        actor.set_actor_label(label)
        actor.set_editor_property("archetype", definition)
        added.append(label)

# Migrate only the known initial fixture reference; custom respawner definitions are retained.
for actor in actors.get_all_level_actors():
    if isinstance(actor, unreal.SandboxEnemyRespawner) and actor.get_editor_property("enemy_definition") == fixture:
        actor.set_editor_property("enemy_definition", definition)
        added.append(actor.get_actor_label() + ":archetype")
respawner = assets.load_asset("/Game/CastleDefender/Core/BP_SandboxEnemyRespawner")
if respawner:
    defaults = unreal.get_default_object(respawner.generated_class())
    if defaults.get_editor_property("enemy_definition") == fixture:
        defaults.set_editor_property("enemy_definition", definition)
        assert unreal.BlueprintEditorLibrary.compile_blueprint(respawner)
        assert assets.save_loaded_asset(respawner, only_if_is_dirty=False)
if added:
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, MAP)
unreal.log("MELEE_CONTENT added=" + str(added) + "; existing actors preserved")
