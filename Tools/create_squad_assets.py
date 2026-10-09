"""T-SQD-01 spawn/cap fixtures. Only creates missing assets; never rewrites existing ones."""
import unreal

ROOT = "/Game/CastleDefender"
ARMY = f"{ROOT}/Army"
TESTS = f"{ROOT}/Maps/Test"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()


def new_blueprint(name, parent):
    path = f"{ARMY}/{name}"
    if assets.does_asset_exist(path):
        return assets.load_asset(path), False
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    blueprint = tools.create_asset(name, ARMY, None, factory)
    assert blueprint, f"Could not create {path}"
    assert unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return blueprint, True


for directory in (ARMY, TESTS):
    if not assets.does_directory_exist(directory):
        assets.make_directory(directory)

soldier_bp, soldier_new = new_blueprint("BP_Soldier_Test", unreal.SoldierCharacter)
if soldier_new:
    cdo = unreal.get_default_object(soldier_bp.generated_class())
    mesh = cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    mannequin = assets.load_asset(f"{ROOT}/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")
    animation = assets.load_asset(f"{ROOT}/Placeholder/Mannequins/Anims/Unarmed/ABP_Unarmed")
    assert mannequin and animation, "Existing licensed mannequin content is required; run Hero placeholder setup first"
    mesh.set_skeletal_mesh_asset(mannequin)
    half_height = cdo.get_editor_property("capsule_component").get_unscaled_capsule_half_height()
    mesh.set_editor_property("relative_location", unreal.Vector(0, 0, -half_height))
    mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0, pitch=0, yaw=-90))
    mesh.set_editor_property("anim_class", animation.generated_class())
    assert unreal.BlueprintEditorLibrary.compile_blueprint(soldier_bp)
    assert assets.save_loaded_asset(soldier_bp)

definition_path = f"{ARMY}/DA_Squad_Test"
if not assets.does_asset_exist(definition_path):
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.SquadDefinition)
    definition = tools.create_asset("DA_Squad_Test", ARMY, unreal.SquadDefinition, factory)
    assert definition
    definition.set_editor_property("display_name", unreal.Text("Infantry spawn fixture"))
    type_tag = unreal.GameplayTag()
    type_tag.import_text('(TagName="Unit.Squad.Infantry")')
    definition.set_editor_property("squad_type_tag", type_tag)
    definition.set_editor_property("soldier_class", soldier_bp.generated_class())
    definition.set_editor_property("soldier_count", 8)
    definition.set_editor_property("formation_columns", 4)
    poise = unreal.CombatStateConfig()
    poise.set_editor_property("max_poise", 50.0)
    definition.set_editor_property("combat_state", poise)
    assert assets.save_loaded_asset(definition)
else:
    definition = assets.load_asset(definition_path)

squad_bp, squad_new = new_blueprint("BP_Squad_Test", unreal.Squad)
if squad_new:
    unreal.get_default_object(squad_bp.generated_class()).set_editor_property("definition", definition)
    assert unreal.BlueprintEditorLibrary.compile_blueprint(squad_bp)
    assert assets.save_loaded_asset(squad_bp)

map_path = f"{TESTS}/L_Test_SquadSpawn"
if not assets.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assert levels.new_level(map_path)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mode = unreal.load_class(None, f"{ROOT}/Core/BP_SandboxGameMode.BP_SandboxGameMode_C")
    assert mode, "Existing combat sandbox game mode is required"
    world.get_world_settings().set_editor_property("default_game_mode", mode)
    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10))
    floor.static_mesh_component.set_static_mesh(assets.load_asset("/Engine/BasicShapes/Cube"))
    floor.set_actor_scale3d(unreal.Vector(60, 60, 0.2))
    floor.set_actor_label("SquadSpawn_Floor")
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1500))
    sun.set_actor_rotation(unreal.Rotator(pitch=-50, yaw=-35, roll=0), False)
    actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 500))
    start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-800, 0, 120))
    start.set_actor_rotation(unreal.Rotator(0, 0, 0), False)
    for index, position in enumerate(((700, -1000), (700, 0), (700, 1000), (1800, 0)), 1):
        squad = actors.spawn_actor_from_class(squad_bp.generated_class(), unreal.Vector(*position, 100))
        squad.set_actor_label(f"SquadSpawn_{index}")
    assert levels.save_current_level()
    unreal.log(f"Created {map_path}; fourth squad is the intentional cap-rejection case")
else:
    unreal.log(f"Preserved existing {map_path}")

unreal.log("SQUAD_ASSETS_COMPLETE: existing assets were preserved")
