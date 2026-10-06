"""Creates enemy content and tests.

T-ENM-01/03: BP_Enemy_Base, DA_Enemy_Test and the placeholder AM_Enemy_Melee_Light/Heavy attack montages.
T-ENM-02: BP_FT_EnemyAggroChase and the FT_Enemy_AggroChase map (functional tests are Blueprint, foundation §16).
Idempotent: existing assets and their edits are kept; only missing assets are created.
DA_Enemy_Test numbers are fixture values from the T-ENM-01 test case, not tuning (that is T-ENM-11).
Run headless: Tools/create_enemy_assets.bat
"""
import runpy
from pathlib import Path
import unreal

ENEMY_DIR = "/Game/CastleDefender/Enemy"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

if not assets.does_directory_exist(ENEMY_DIR):
    assets.make_directory(ENEMY_DIR)

# Placeholder Mannequin content (source/license: Placeholder/LICENSES.md).
placeholder = runpy.run_path(str(Path(__file__).with_name("create_hero_placeholder_content.py")))
placeholder["ensure_mannequin"]()
MANNEQUIN = placeholder["MANNEQUIN"]

# 1. Placeholder attack montages (T-ENM-03). Wind-ups are user-approved placeholders (2026-10-06): Light 0.5 s,
#    Heavy 0.8 s, both above the 0.4 s minimum telegraph. Clips are stretched/trimmed so the strike lands in the window.
def attack_montage(name, clip_path, clip_start, strike_in_clip, wind_up, hit_length):
    path = f"{ENEMY_DIR}/{name}"
    if assets.does_asset_exist(path):
        return assets.load_asset(path)
    source = placeholder["anim"](clip_path)
    factory = unreal.AnimMontageFactory()
    factory.set_editor_property("target_skeleton", source.get_editor_property("skeleton"))
    factory.set_editor_property("source_animation", source)
    montage = asset_tools.create_asset(name, ENEMY_DIR, unreal.AnimMontage, factory)
    scale = wind_up / (strike_in_clip - clip_start)
    duration = (source.get_play_length() - clip_start) * scale
    assert unreal.HeroCombatLibrary.set_single_segment_montage_source(montage, source, clip_start, source.get_play_length(), duration, False)
    unreal.HeroCombatLibrary.clear_combat_notifies_from_montage(montage)
    assert unreal.HeroCombatLibrary.add_combat_hit_window_to_montage(montage, wind_up, hit_length), f"{name}: hit window rejected"
    assets.save_loaded_asset(montage, only_if_is_dirty=False)
    unreal.log(f"Created {path}")
    return montage


# Strike times measured for the hero (create_hero_assets.py): MM_Attack_01 at 0.28 s, MM_ChargedAttack at 1.0 s.
light = attack_montage("AM_Enemy_Melee_Light", "Unarmed/Attack/MM_Attack_01", 0.0, 0.28, 0.5, 0.2)
heavy = attack_montage("AM_Enemy_Melee_Heavy", "Unarmed/Attack/MM_ChargedAttack", 0.2, 1.0, 0.8, 0.22)


def attack(montage, damage, poise, heavy_flag, cooldown, weight):
    """Fixture numbers for DA_Enemy_Test, not tuning (T-ENM-11 authors DA_Enemy_Melee)."""
    result = unreal.EnemyAttackDefinition()
    result.set_editor_property("montage", montage)
    result.set_editor_property("range", 150.0)
    result.set_editor_property("damage", damage)
    result.set_editor_property("poise_damage", poise)
    result.set_editor_property("is_heavy", heavy_flag)
    result.set_editor_property("cooldown", cooldown)
    result.set_editor_property("weight", weight)
    return result


TEST_ATTACKS = [attack(light, 10.0, 10.0, False, 0.0, 2.0), attack(heavy, 20.0, 25.0, True, 3.0, 1.0)]

# 2. BP_Enemy_Base. Archetype stays unset to avoid a DA/BP reference cycle; level instances set it.
bp_path = f"{ENEMY_DIR}/BP_Enemy_Base"
if assets.does_asset_exist(bp_path):
    enemy_bp = assets.load_asset(bp_path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.load_class(None, "/Script/CastleDefender.EnemyCharacter"))
    enemy_bp = asset_tools.create_asset("BP_Enemy_Base", ENEMY_DIR, None, factory)
    cdo = unreal.get_default_object(enemy_bp.generated_class())
    mesh = cdo.get_editor_property("mesh")
    half_height = cdo.get_editor_property("capsule_component").get_unscaled_capsule_half_height()
    # Quinn, so the enemy reads differently from the hero at a glance.
    mesh.set_skeletal_mesh_asset(assets.load_asset(f"{MANNEQUIN}/Meshes/SKM_Quinn_Simple"))
    mesh.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, -half_height))
    mesh.set_editor_property("relative_rotation", unreal.Rotator(roll=0.0, pitch=0.0, yaw=-90.0))
    mesh.set_editor_property("anim_class", assets.load_asset(f"{MANNEQUIN}/Anims/Unarmed/ABP_Unarmed").generated_class())
    unreal.BlueprintEditorLibrary.compile_blueprint(enemy_bp)
    assets.save_loaded_asset(enemy_bp, only_if_is_dirty=False)
    unreal.log(f"Created {bp_path}")

# 3. DA_Enemy_Test.
da_path = f"{ENEMY_DIR}/DA_Enemy_Test"
if not assets.does_asset_exist(da_path):
    def_class = unreal.load_class(None, "/Script/CastleDefender.EnemyArchetypeDefinition")
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", def_class)
    da = asset_tools.create_asset("DA_Enemy_Test", ENEMY_DIR, def_class, factory)
    da.set_editor_property("display_name", unreal.Text("Test Enemy"))
    da.set_editor_property("enemy_class", enemy_bp.generated_class())
    da.set_editor_property("max_health", 100.0)
    da.set_editor_property("walk_speed", 300.0)
    combat_state = da.get_editor_property("combat_state")
    combat_state.set_editor_property("max_poise", 50.0)
    da.set_editor_property("combat_state", combat_state)
    da.set_editor_property("attacks", TEST_ATTACKS)
    da.set_editor_property("wind_up_turn_rate", 360.0)
    da.set_editor_property("min_time_between_attacks", 1.0)
    assets.save_loaded_asset(da, only_if_is_dirty=False)
    unreal.log(f"Created {da_path}")

# 4. T-ENM-02 functional test: hero outside the aggro radius → Idle; hero inside → targeted and approached.
TEST_MAPS = "/Game/CastleDefender/Maps/Test"
HERO_CLASS = "/Game/CastleDefender/Hero/BP_Hero_Warlord.BP_Hero_Warlord_C"
ft_path = f"{TEST_MAPS}/BP_FT_EnemyAggroChase"
if assets.does_asset_exist(ft_path):
    ft_bp = assets.load_asset(ft_path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.FunctionalTest)
    ft_bp = asset_tools.create_asset("BP_FT_EnemyAggroChase", TEST_MAPS, None, factory)
    unreal.BlueprintEditorLibrary.add_event_override(ft_bp, "ReceiveStartTest", unreal.IntPoint(0, 0))


def wire_aggro_chase(bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(unreal.BlueprintEditorLibrary.find_event_graph(bp))
    start = editor.find_event_node("ReceiveStartTest")
    if start.find_then_pin().list_connected_pins():
        return  # Preserve an already wired graph, including user edits.

    def call(path, x, y=0):
        node = editor.add_call_function_node(path)
        if node is None:
            raise RuntimeError(f"Cannot create {path}")
        node.set_node_pos(unreal.IntPoint(x, y))
        return node

    def link(output, input_pin):
        if not output.try_create_connection(input_pin):
            raise RuntimeError(f"Aggro chase graph: cannot connect {output.get_pin_name()} -> {input_pin.get_pin_name()}")

    def value(pin, literal):
        if not pin.set_pin_value(literal):
            raise RuntimeError(f"Aggro chase graph: invalid value {literal}")

    def spawn(cls, x, transform):
        node = editor.create_node_from_name("Game|SpawnActorfromClass", unreal.Vector2D(x, 0), [])
        value(node.find_input_pin("Class"), cls)
        value(node.find_input_pin("CollisionHandlingOverride"), "AlwaysSpawn")
        link(transform, node.find_input_pin("SpawnTransform"))
        return node

    def offset(x, y, dx):
        """Location of this test actor + (dx, 0, 0)."""
        here = call("/Script/Engine.Actor.K2_GetActorLocation", x, y)
        add = call("/Script/Engine.KismetMathLibrary.Add_VectorVector", x + 220, y)
        link(here.find_result_pin(), add.find_input_pin("A"))
        value(add.find_input_pin("B"), f"{dx},0,0")
        return add.find_result_pin()

    def state_is(enemy, literal, x, y):
        brain = call("/Script/CastleDefender.EnemyCharacter.GetBrainComponent", x, y)
        link(enemy, brain.find_self_pin())
        state = call("/Script/CastleDefender.EnemyBrainComponent.GetState", x + 250, y)
        link(brain.find_result_pin(), state.find_self_pin())
        equal = call("/Script/Engine.KismetMathLibrary.EqualEqual_ByteByte", x + 500, y)
        link(state.find_result_pin(), equal.find_input_pin("A"))
        value(equal.find_input_pin("B"), literal)
        return equal.find_result_pin()

    def finish(source, result, message, x, y):
        node = call("/Script/FunctionalTesting.FunctionalTest.FinishTest", x, y)
        value(node.find_input_pin("TestResult"), result)
        value(node.find_input_pin("Message"), message)
        link(source, node.find_execute_pin())

    def delay(source, seconds, x):
        node = call("/Script/Engine.KismetSystemLibrary.Delay", x)
        value(node.find_input_pin("Duration"), str(seconds))
        link(source, node.find_execute_pin())
        return node

    # Enemy at this actor, hero 10 m away (aggro radius 6 m in DA_Enemy_Test).
    here = call("/Script/Engine.Actor.GetTransform", 0, 300)
    enemy = spawn(f"{ENEMY_DIR}/BP_Enemy_Base.BP_Enemy_Base_C", 300, here.find_result_pin())
    value(enemy.find_input_pin("Archetype"), f"{ENEMY_DIR}/DA_Enemy_Test.DA_Enemy_Test")
    link(start.find_then_pin(), enemy.find_execute_pin())
    far = call("/Script/Engine.KismetMathLibrary.MakeTransform", 900, 300)
    link(offset(400, 450, 1000.0), far.find_input_pin("Location"))
    hero = spawn(HERO_CLASS, 800, far.find_result_pin())
    link(enemy.find_then_pin(), hero.find_execute_pin())
    enemy_ref = enemy.find_result_pin()

    # 1 s covers several 0.2 s decisions.
    wait_idle = delay(hero.find_then_pin(), 1.0, 1200)
    idle = editor.add_branch_node()
    idle.set_node_pos(unreal.IntPoint(1500, 0))
    link(wait_idle.find_then_pin(), idle.find_execute_pin())
    link(state_is(enemy_ref, "0", 1100, 300), idle.find_condition_pin())  # EEnemyBrainState::Idle
    finish(idle.find_else_pin(), "Failed", "Enemy left Idle with the hero outside its aggro radius", 1750, 400)

    move = call("/Script/Engine.Actor.K2_SetActorLocation", 1750)
    link(hero.find_result_pin(), move.find_self_pin())
    link(offset(1500, 600, 300.0), move.find_input_pin("NewLocation"))
    value(move.find_input_pin("bTeleport"), "true")
    link(idle.find_then_pin(), move.find_execute_pin())

    wait_engage = delay(move.find_then_pin(), 1.0, 2100)
    # Closer than the 300 cm gap it was given means it walked toward the hero (it may already have arrived and stopped).
    gap = call("/Script/Engine.Actor.GetDistanceTo", 2000, 450)
    link(enemy_ref, gap.find_self_pin())
    link(hero.find_result_pin(), gap.find_input_pin("OtherActor"))
    moving = call("/Script/Engine.KismetMathLibrary.Less_DoubleDouble", 2250, 450)
    link(gap.find_result_pin(), moving.find_input_pin("A"))
    value(moving.find_input_pin("B"), "280")
    both = call("/Script/Engine.KismetMathLibrary.BooleanAND", 2700, 300)
    # Engaged = the brain targets the hero (it may already be Attacking once in range, T-ENM-03).
    brain = call("/Script/CastleDefender.EnemyCharacter.GetBrainComponent", 2000, 300)
    link(enemy_ref, brain.find_self_pin())
    target = call("/Script/CastleDefender.EnemyBrainComponent.GetTarget", 2250, 300)
    link(brain.find_result_pin(), target.find_self_pin())
    same = call("/Script/Engine.KismetMathLibrary.EqualEqual_ObjectObject", 2450, 300)
    link(target.find_result_pin(), same.find_input_pin("A"))
    link(hero.find_result_pin(), same.find_input_pin("B"))
    link(same.find_result_pin(), both.find_input_pin("A"))
    link(moving.find_result_pin(), both.find_input_pin("B"))
    engaged = editor.add_branch_node()
    engaged.set_node_pos(unreal.IntPoint(2900, 0))
    link(wait_engage.find_then_pin(), engaged.find_execute_pin())
    link(both.find_result_pin(), engaged.find_condition_pin())
    finish(engaged.find_then_pin(), "Succeeded", "Enemy engaged and moved toward the hero", 3150, 0)
    finish(engaged.find_else_pin(), "Failed", "Enemy did not engage and move toward the hero", 3150, 200)

    if not unreal.BlueprintEditorLibrary.compile_blueprint(bp) or editor.list_nodes_with_errors() or editor.list_nodes_with_warnings():
        raise RuntimeError("BP_FT_EnemyAggroChase did not compile without warnings")
    assets.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log("Wired and saved BP_FT_EnemyAggroChase")


wire_aggro_chase(ft_bp)

map_path = f"{TEST_MAPS}/FT_Enemy_AggroChase"
if not assets.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(map_path):
        raise RuntimeError(f"Failed to create {map_path}")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    floor.static_mesh_component.set_static_mesh(assets.load_asset("/Engine/BasicShapes/Cube"))
    floor.set_actor_scale3d(unreal.Vector(40.0, 40.0, 1.0))
    floor.set_actor_label("Floor")
    bounds = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 0))
    bounds.set_actor_scale3d(unreal.Vector(20.0, 20.0, 5.0))
    # Navmesh: built and saved by create_enemy_assets.ps1 (ResavePackages -BuildNavigationData).
    test = actors.spawn_actor_from_class(ft_bp.generated_class(), unreal.Vector(-500, 0, 100))
    test.set_actor_label("FT_Enemy_AggroChase")
    if not levels.save_current_level():
        raise RuntimeError(f"Failed to save {map_path}")
    unreal.log(f"Created {map_path}")
