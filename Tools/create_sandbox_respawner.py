"""T-CMB-14: add Duel/Pair presets without clearing or rebuilding the sandbox map.

Existing Blueprint tuning and existing preset instances are preserved.
Run: Tools/create_sandbox_respawner.bat
"""
import unreal

core = "/Game/CastleDefender/Core"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()
path = core + "/BP_SandboxEnemyRespawner"
if assets.does_asset_exist(path):
    bp = assets.load_asset(path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.SandboxEnemyRespawner)
    bp = tools.create_asset("BP_SandboxEnemyRespawner", core, None, factory)
    defaults = unreal.get_default_object(bp.generated_class())
    definition = assets.load_asset("/Game/CastleDefender/Enemy/DA_Enemy_Test")
    assert definition is not None, "Create the P0 enemy content first"
    defaults.set_editor_property("enemy_definition", definition)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert assets.save_loaded_asset(bp, only_if_is_dirty=False)

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
assert world
existing = {actor.get_actor_label() for actor in actors.get_all_level_actors()}
added = []
for label, count, enabled, x in (("Duel", 1, True, 500), ("Pair", 2, False, 700)):
    if label in existing:
        continue
    actor = actors.spawn_actor_from_class(bp.generated_class(), unreal.Vector(x, -1500, 0))
    assert actor
    actor.set_actor_label(label)
    actor.set_editor_property("count", count)
    actor.set_editor_property("enabled", enabled)
    actor.set_editor_property("spawn_radius", 150.0)
    added.append(label)
if added:
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/CastleDefender/Maps/L_CombatSandbox")
unreal.log("SANDBOX_RESPAWNER added=" + str(added) + "; existing actors preserved")
