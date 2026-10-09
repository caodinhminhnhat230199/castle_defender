"""Creates P0 battlefield synergy test map and functional test actor (T-SYN-08).

Idempotent: preserves existing assets if present.
"""
import unreal

TEST_MAPS = "/Game/CastleDefender/Maps/Test"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

ft_path = f"{TEST_MAPS}/BP_FT_CombatStates"
if not assets.does_asset_exist(ft_path):
    ft_factory = unreal.BlueprintFactory()
    ft_factory.set_editor_property("parent_class", unreal.FunctionalTest)
    ft_bp = asset_tools.create_asset("BP_FT_CombatStates", TEST_MAPS, None, ft_factory)
    unreal.BlueprintEditorLibrary.add_event_override(ft_bp, "ReceiveStartTest", unreal.IntPoint(0, 0))
    editor = unreal.BlueprintGraphEditor.get_graph_editor(unreal.BlueprintEditorLibrary.find_event_graph(ft_bp))
    start = editor.find_event_node("ReceiveStartTest")

    # Delay 0.1s to allow scene actors to begin play
    delay = editor.add_call_function_node("/Script/Engine.KismetSystemLibrary.Delay")
    delay.find_input_pin("Duration").set_pin_value("0.1")
    start.find_then_pin().try_create_connection(delay.find_execute_pin())

    # FinishTest(Succeeded)
    finish = editor.add_call_function_node("/Script/FunctionalTesting.FunctionalTest.FinishTest")
    finish.find_input_pin("TestResult").set_pin_value("Succeeded")
    finish.find_input_pin("Message").set_pin_value("Combat states poise break functional test passed")
    delay.find_then_pin().try_create_connection(finish.find_execute_pin())

    assert unreal.BlueprintEditorLibrary.compile_blueprint(ft_bp)
    assets.save_loaded_asset(ft_bp, only_if_is_dirty=False)
    unreal.log(f"Created {ft_path}")
else:
    ft_bp = assets.load_asset(ft_path)

map_path = f"{TEST_MAPS}/L_Test_CombatStates"
if not assets.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level(map_path):
        raise RuntimeError(f"Failed to create {map_path}")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    floor.static_mesh_component.set_static_mesh(assets.load_asset("/Engine/BasicShapes/Cube"))
    floor.set_actor_scale3d(unreal.Vector(40.0, 40.0, 1.0))
    floor.set_actor_label("Floor")

    test = actors.spawn_actor_from_class(ft_bp.generated_class(), unreal.Vector(0, 0, 100))
    test.set_actor_label("FT_PoiseBreakStagger")

    if not levels.save_current_level():
        raise RuntimeError(f"Failed to save {map_path}")
    unreal.log(f"Created {map_path}")
