"""Creates P0 Enemy Combat functional test map and functional test actors (T-ENM-12).

Idempotent: preserves existing assets if present.
"""
import unreal

TEST_MAPS = "/Game/CastleDefender/Maps/Test"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

scenarios = [
    ("BP_FT_Enemy_AggroChase", "FT_Enemy_AggroChase"),
    ("BP_FT_Enemy_TelegraphGap", "FT_Enemy_TelegraphGap"),
    ("BP_FT_Enemy_StaggerCancel", "FT_Enemy_StaggerCancel"),
    ("BP_FT_Enemy_DeathReportOnce", "FT_Enemy_DeathReportOnce"),
    ("BP_FT_Enemy_ParryStaggers", "FT_Enemy_ParryStaggers"),
]

for bp_name, scenario_id in scenarios:
    ft_path = f"{TEST_MAPS}/{bp_name}"
    if not assets.does_asset_exist(ft_path):
        ft_factory = unreal.BlueprintFactory()
        ft_factory.set_editor_property("parent_class", unreal.FunctionalTest)
        ft_bp = asset_tools.create_asset(bp_name, TEST_MAPS, None, ft_factory)
        unreal.BlueprintEditorLibrary.add_event_override(ft_bp, "ReceiveStartTest", unreal.IntPoint(0, 0))
        editor = unreal.BlueprintGraphEditor.get_graph_editor(unreal.BlueprintEditorLibrary.find_event_graph(ft_bp))
        start = editor.find_event_node("ReceiveStartTest")

        # Delay 0.1s to allow scene actors to begin play
        delay = editor.add_call_function_node("/Script/Engine.KismetSystemLibrary.Delay")
        delay.find_input_pin("Duration").set_pin_value("0.1")
        start.find_then_pin().try_create_connection(delay.find_execute_pin())

        # RunEnemyCombatScenario
        run_node = editor.add_call_function_node("/Script/CastleDefender.EnemyCombatTestLibrary.RunEnemyCombatScenario")
        run_node.find_input_pin("ScenarioName").set_pin_value(scenario_id)
        delay.find_then_pin().try_create_connection(run_node.find_execute_pin())

        # Branch
        branch = editor.add_branch_node()
        run_node.find_then_pin().try_create_connection(branch.find_execute_pin())

        # Find ReturnValue and OutMessage pins explicitly
        ret_pin = None
        msg_pin = None
        for p in run_node.list_all_pins():
            pname = p.get_pin_name()
            if pname == "ReturnValue":
                ret_pin = p
            elif pname == "OutMessage":
                msg_pin = p

        assert ret_pin and msg_pin, f"Pins on {bp_name} run_node not found"
        ret_pin.try_create_connection(branch.find_condition_pin())

        # FinishTest(Succeeded)
        finish_pass = editor.add_call_function_node("/Script/FunctionalTesting.FunctionalTest.FinishTest")
        finish_pass.find_input_pin("TestResult").set_pin_value("Succeeded")
        msg_pin.try_create_connection(finish_pass.find_input_pin("Message"))
        branch.find_then_pin().try_create_connection(finish_pass.find_execute_pin())

        # FinishTest(Failed)
        finish_fail = editor.add_call_function_node("/Script/FunctionalTesting.FunctionalTest.FinishTest")
        finish_fail.find_input_pin("TestResult").set_pin_value("Failed")
        msg_pin.try_create_connection(finish_fail.find_input_pin("Message"))
        branch.find_else_pin().try_create_connection(finish_fail.find_execute_pin())

        assert unreal.BlueprintEditorLibrary.compile_blueprint(ft_bp)
        assets.save_loaded_asset(ft_bp, only_if_is_dirty=False)
        unreal.log(f"Created {ft_path}")
    else:
        unreal.log(f"Asset {ft_path} already exists")

map_path = f"{TEST_MAPS}/L_Test_EnemyCombat"
if not assets.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(map_path):
        raise RuntimeError(f"Failed to create {map_path}")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    floor.static_mesh_component.set_static_mesh(assets.load_asset("/Engine/BasicShapes/Cube"))
    floor.set_actor_scale3d(unreal.Vector(50.0, 50.0, 1.0))
    floor.set_actor_label("Floor")

    bounds = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 0))
    bounds.set_actor_scale3d(unreal.Vector(25.0, 25.0, 5.0))
    bounds.set_actor_label("NavMeshBoundsVolume")

    hero_cls = unreal.load_class(None, "/Game/CastleDefender/Hero/BP_Hero_Warlord.BP_Hero_Warlord_C")
    if hero_cls:
        hero = actors.spawn_actor_from_class(hero_cls, unreal.Vector(0, 0, 100))
        hero.set_actor_label("Hero_Warlord")

    for i, (bp_name, scenario_id) in enumerate(scenarios):
        bp_asset = assets.load_asset(f"{TEST_MAPS}/{bp_name}")
        test_actor = actors.spawn_actor_from_class(bp_asset.generated_class(), unreal.Vector(i * 100.0, 300.0, 100.0))
        test_actor.set_actor_label(scenario_id)

    if not levels.save_current_level():
        raise RuntimeError(f"Failed to save {map_path}")
    unreal.log(f"Created {map_path}")
else:
    unreal.log(f"Map {map_path} already exists")
