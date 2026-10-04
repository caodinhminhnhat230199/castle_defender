"""Author the T-CMB-11 sandbox-only respawn graph using UE's existing editor APIs."""
import unreal


def wire_sandbox_respawn(bp):
    library = unreal.BlueprintEditorLibrary
    graph = unreal.BlueprintGraphEditor
    event_editor = graph.get_graph_editor(library.find_event_graph(bp))
    if library.find_graph(bp, 'SandboxHeroDied') is not None:
        return  # Preserve the authored graph and designer tuning on repeat runs.
    if any(p.list_connected_pins() for n in event_editor.list_all_nodes() for p in n.list_all_pins()):
        raise RuntimeError('Sandbox game mode has custom wiring; refusing to overwrite it')

    def link(source, target):
        if not source.try_create_connection(target):
            raise RuntimeError('Cannot connect sandbox respawn pins')

    def value(pin, literal):
        if not pin.set_pin_value(literal):
            raise RuntimeError('Cannot set sandbox respawn value: ' + literal)

    def call(editor, path, x, y=0):
        node = editor.add_call_function_node(path)
        if node is None:
            raise RuntimeError('Cannot create ' + path)
        node.set_node_pos(unreal.IntPoint(x, y))
        return node

    event_editor.add_member_variable('RespawnDelay', library.get_basic_type_by_name('real'), '3.0')
    library.set_blueprint_variable_instance_editable(bp, 'RespawnDelay', True)
    library.set_blueprint_variable_category(bp, 'RespawnDelay', unreal.Text('Sandbox'))

    died = graph.create_and_edit_function_graph(bp, 'SandboxHeroDied')
    died.add_graph_input_parameter('KillingHit', library.get_struct_type(unreal.CombatHit.static_struct()))
    timer = call(died, '/Script/Engine.KismetSystemLibrary.K2_SetTimer', 300)
    value(timer.find_input_pin('FunctionName'), 'SandboxRespawn')
    delay = died.add_get_member_variable_node('RespawnDelay')
    link(delay.find_output_pin('RespawnDelay'), timer.find_input_pin('Time'))
    link(died.find_graph_entry_pin(), timer.find_execute_pin())

    respawn = graph.create_and_edit_function_graph(bp, 'SandboxRespawn')
    controller = call(respawn, '/Script/Engine.GameplayStatics.GetPlayerController', 0, 220)
    pawn = call(respawn, '/Script/Engine.Controller.K2_GetPawn', 240, 220)
    link(controller.find_result_pin(), pawn.find_self_pin())
    destroy = call(respawn, '/Script/Engine.Actor.K2_DestroyActor', 320)
    link(pawn.find_result_pin(), destroy.find_self_pin())
    link(respawn.find_graph_entry_pin(), destroy.find_execute_pin())
    restart = call(respawn, '/Script/Engine.GameModeBase.RestartPlayer', 600)
    link(controller.find_result_pin(), restart.find_input_pin('NewPlayer'))
    link(destroy.find_then_pin(), restart.find_execute_pin())

    # OnRestartPlayer fires after possession, including subsequent sandbox respawns.
    started = library.add_event_override(bp, 'K2_OnRestartPlayer', unreal.IntPoint(0, 0))
    if started is None:
        raise RuntimeError('Cannot override OnRestartPlayer')
    new_player = started.find_output_pin('NewPlayer')
    pawn = call(event_editor, '/Script/Engine.Controller.K2_GetPawn', 0, 200)
    link(new_player, pawn.find_self_pin())
    cast = event_editor.create_node_from_name('Utilities|Casting|CastToHeroCharacter', unreal.Vector2D(300, 0), [pawn.find_result_pin()])
    link(pawn.find_result_pin(), cast.find_input_pin('Object'))
    link(started.find_then_pin(), cast.find_execute_pin())
    bind = event_editor.create_node_from_name('Hero|BindEventtoOnHeroDeath', unreal.Vector2D(600, 0), [])
    link(cast.find_output_pin('AsHero Character'), bind.find_self_pin())
    link(cast.find_then_pin(), bind.find_execute_pin())
    event_pin = bind.find_input_pin('Delegate')
    available = event_editor.list_available_nodes([event_pin])
    event_name = next((str(n) for n in available if str(n).endswith('CreateEvent')), None)
    if event_name is None:
        raise RuntimeError('Create Event node unavailable: ' + str(available))
    delegate = event_editor.create_node_from_name(event_name, unreal.Vector2D(600, 220), [event_pin])
    link(delegate.find_output_pin('OutputDelegate'), event_pin)
    library.set_create_delegate_function(delegate, 'SandboxHeroDied')
    # Sandbox only: world game time. CSM T-CSM-01 replaces this binding in P3.

    if not library.compile_blueprint(bp):
        raise RuntimeError('Sandbox respawn Blueprint did not compile')
    for editor in [event_editor, died, respawn]:
        if editor.list_nodes_with_errors() or editor.list_nodes_with_warnings():
            raise RuntimeError('Sandbox respawn graph has warnings or errors')
    unreal.get_editor_subsystem(unreal.EditorAssetSubsystem).save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log('Wired sandbox respawn: OnHeroDeath -> world timer -> destroy pawn -> RestartPlayer')


if __name__ == '__main__':
    wire_sandbox_respawn(unreal.load_asset('/Game/CastleDefender/Core/BP_SandboxGameMode'))
