"""Creates the P0 Combat Sandbox assets: BP_SandboxGameMode and L_CombatSandbox.

Idempotent: can be run headless or in the editor.
Run headless: Tools/create_combat_sandbox.bat
"""
import gc
import unreal

ROOT = "/Game/CastleDefender"
CORE = f"{ROOT}/Core"
MAPS = f"{ROOT}/Maps"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# 1. BP_SandboxGameMode
gamemode_path = f"{CORE}/BP_SandboxGameMode"
controller_class = unreal.load_class(None, f"{CORE}/BP_HeroPlayerController.BP_HeroPlayerController_C")

if not assets.does_asset_exist(gamemode_path):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.GameModeBase)
    gm_bp = asset_tools.create_asset("BP_SandboxGameMode", CORE, None, factory)
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    gm_cdo.set_editor_property("player_controller_class", controller_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(gm_bp)
    assets.save_loaded_asset(gm_bp, only_if_is_dirty=False)
    unreal.log("Created BP_SandboxGameMode")
else:
    gm_bp = assets.load_asset(gamemode_path)
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    gm_cdo.set_editor_property("player_controller_class", controller_class)
    unreal.BlueprintEditorLibrary.compile_blueprint(gm_bp)
    assets.save_loaded_asset(gm_bp, only_if_is_dirty=False)
    unreal.log("Updated BP_SandboxGameMode")

gm_class = gm_bp.generated_class()
del gm_bp, gm_cdo
gc.collect()

# 2. L_CombatSandbox Map
map_name = "L_CombatSandbox"
map_package = f"{MAPS}/{map_name}"

if not assets.does_asset_exist(map_package):
    world_factory = unreal.WorldFactory()
    world = asset_tools.create_asset(map_name, MAPS, unreal.World, world_factory)
    assets.save_loaded_asset(world, only_if_is_dirty=False)
    del world
    gc.collect()
    unreal.log(f"Created map asset {map_package}")

loaded_world = unreal.EditorLoadingAndSavingUtils.load_map(map_package)
world_settings = loaded_world.get_world_settings()
world_settings.set_editor_property("default_game_mode", gm_class)

# Clear existing non-system actors in the map
for a in actors.get_all_level_actors():
    if isinstance(a, (unreal.WorldSettings, unreal.Brush)):
        continue
    actors.destroy_actor(a)

cube_mesh = unreal.load_object(None, "/Engine/BasicShapes/Cube.Cube")
cylinder_mesh = unreal.load_object(None, "/Engine/BasicShapes/Cylinder.Cylinder")

# Flat arena floor: 60m x 60m (6000 x 6000 x 20 cm)
floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10))
floor.set_actor_label("Floor_Arena_60x60m")
floor.static_mesh_component.set_static_mesh(cube_mesh)
floor.set_actor_scale3d(unreal.Vector(60.0, 60.0, 0.2))

# Cover pillars for LOS checks
p1 = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(-1000, 1000, 150))
p1.set_actor_label("Pillar_LOS_1")
p1.static_mesh_component.set_static_mesh(cylinder_mesh)
p1.set_actor_scale3d(unreal.Vector(2.0, 2.0, 3.0))

p2 = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1000, 1000, 150))
p2.set_actor_label("Pillar_LOS_2")
p2.static_mesh_component.set_static_mesh(cylinder_mesh)
p2.set_actor_scale3d(unreal.Vector(2.0, 2.0, 3.0))

# Ramp for elevation/angle checks
ramp = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(-1500, 0, 100), unreal.Rotator(15, 0, 0))
ramp.set_actor_label("Ramp_LOS_1")
ramp.static_mesh_component.set_static_mesh(cube_mesh)
ramp.set_actor_scale3d(unreal.Vector(5.0, 15.0, 0.2))

# Wall for camera collision checks
wall = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 2500, 200))
wall.set_actor_label("Wall_CameraCollision")
wall.static_mesh_component.set_static_mesh(cube_mesh)
wall.set_actor_scale3d(unreal.Vector(12.0, 0.5, 4.0))

# PlayerStart
ps = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, -1500, 100), unreal.Rotator(0, 90, 0))
ps.set_actor_label("PlayerStart")

# Lighting environment
sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, -45, 0))
sun.set_actor_label("DirectionalLight")

sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 500))
sky.set_actor_label("SkyLight")

sky_atmo = actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 500))
sky_atmo.set_actor_label("SkyAtmosphere")

fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 100))
fog.set_actor_label("ExponentialHeightFog")

# NavMeshBoundsVolume for enemy pathfinding
nav = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 200))
nav.set_actor_label("NavMeshBoundsVolume")
nav.set_actor_scale3d(unreal.Vector(70.0, 70.0, 10.0))

# 3 Test Dummies: two hostile (team 1), one ally (team 0)
dummy_class = unreal.load_class(None, "/Script/CastleDefender.TestDummy")

d1 = actors.spawn_actor_from_class(dummy_class, unreal.Vector(0, 500, 0))
d1.set_actor_label("TestDummy_Hostile_1")

d2 = actors.spawn_actor_from_class(dummy_class, unreal.Vector(600, 800, 0))
d2.set_actor_label("TestDummy_Hostile_2")

d3 = actors.spawn_actor_from_class(dummy_class, unreal.Vector(-600, 800, 0))
d3.set_actor_label("TestDummy_Ally_1")
team_player = unreal.GenericTeamId()
team_player.set_editor_property("team_id", 0)
d3.set_editor_property("team_id", team_player)

# Tuning Kiosk (TextRenderActor)
kiosk = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(-1000, -1200, 120), unreal.Rotator(0, 45, 0))
kiosk.set_actor_label("TuningKiosk_Text")
kiosk_comp = kiosk.text_render
kiosk_comp.set_editor_property("text", unreal.Text("COMBAT SANDBOX CHEATS\n\ngame.debug.Combat 1\ngame.debug.CombatTrace 1\nSpawnTestDummy\nReloadHeroTuning\nInfiniteStamina\nKillHero\n\nActive DA: DA_HeroClass_Warlord"))
kiosk_comp.set_editor_property("world_size", 24.0)

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("Saved L_CombatSandbox successfully")
