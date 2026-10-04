"""Creates the Phase F editor assets that cannot be written as text.

Idempotent: existing content is preserved; the empty Foundation smoke graph is completed once.
Run headless: Tools/create_foundation_assets.bat
Or in the editor: Tools > Execute Python Script.
"""
import unreal

ROOT = "/Game/CastleDefender"
CORE = f"{ROOT}/Core"
INPUT = f"{CORE}/Input"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)


def get_or_create(name, path, asset_class, factory, setup=None):
    full = f"{path}/{name}"
    if assets.does_asset_exist(full):
        return assets.load_asset(full)
    asset = asset_tools.create_asset(name, path, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {full}")
    if setup:
        setup(asset)
    assets.save_loaded_asset(asset, only_if_is_dirty=False)
    unreal.log(f"Created {full}")
    return asset


def blueprint(name, path, parent, setup):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    return get_or_create(name, path, None, factory,
                         lambda bp: setup(unreal.get_default_object(bp.generated_class())))


# T-FND-01: empty boot map.
get_or_create("L_Boot", f"{ROOT}/Maps", unreal.World, unreal.WorldFactory())

# T-FND-06: input actions and mapping contexts (ai/game/00-foundation/input-keymap.md).
V = unreal.InputActionValueType
ACTIONS = {
    "IA_Move": V.AXIS2D, "IA_Look": V.AXIS2D, "IA_Sprint": V.BOOLEAN,
    "IA_LightAttack": V.BOOLEAN, "IA_HeavyAttack": V.BOOLEAN, "IA_Dodge": V.BOOLEAN,
    "IA_Block": V.BOOLEAN, "IA_LockOn": V.BOOLEAN, "IA_Interact": V.BOOLEAN,
    "IA_DebugToggleBuild": V.BOOLEAN,
}
ia = {name: get_or_create(name, INPUT, unreal.InputAction, unreal.InputAction_Factory(),
                          lambda a, vt=vt: a.set_editor_property("value_type", vt))
      for name, vt in ACTIONS.items()}


def negate(outer, x=True, y=True):
    m = unreal.new_object(unreal.InputModifierNegate, outer)
    m.set_editor_property("x", x)
    m.set_editor_property("y", y)
    m.set_editor_property("z", False)
    return m


def swizzle(outer):
    return unreal.new_object(unreal.InputModifierSwizzleAxis, outer)  # default order YXZ


def key(name):
    k = unreal.Key()
    k.import_text(name)
    return k


def mappings(entries):
    def setup(imc):
        rows = []
        for action, key_name, modifiers in entries:
            row = unreal.EnhancedActionKeyMapping()
            row.set_editor_property("action", ia[action])
            row.set_editor_property("key", key(key_name))
            row.set_editor_property("modifiers", [m(imc) for m in modifiers])
            rows.append(row)
        data = unreal.InputMappingContextMappingData()
        data.set_editor_property("mappings", rows)
        imc.set_editor_property("default_key_mappings", data)
    return setup


IMC_ENTRIES = {
    "IMC_Combat": [
        ("IA_Move", "W", [swizzle]),
        ("IA_Move", "S", [swizzle, negate]),
        ("IA_Move", "A", [negate]),
        ("IA_Move", "D", []),
        ("IA_Look", "Mouse2D", [lambda o: negate(o, x=False, y=True)]),
        ("IA_Sprint", "LeftShift", []),
        ("IA_LightAttack", "LeftMouseButton", []),
        ("IA_HeavyAttack", "RightMouseButton", []),
        ("IA_Dodge", "SpaceBar", []),
        ("IA_Block", "LeftControl", []),
        ("IA_LockOn", "MiddleMouseButton", []),
        ("IA_Interact", "F", []),
    ],
    "IMC_CommandWheel": [],
    "IMC_TacticalFocus": [],
    "IMC_CommanderSpirit": [],
    "IMC_Build": [],
    "IMC_Debug": [("IA_DebugToggleBuild", "F5", [])],
}
imc = {name: get_or_create(name, INPUT, unreal.InputMappingContext, unreal.InputMappingContext_Factory(),
                           mappings(entries))
       for name, entries in IMC_ENTRIES.items()}


def setup_controller(cdo):
    def ctx(name, priority=0):
        c = unreal.PlayerModeMappingContext()
        c.set_editor_property("mapping_context", imc[name])
        c.set_editor_property("priority", priority)
        return c

    def mode(contexts, show_cursor=False):
        m = unreal.PlayerModeInput()
        m.set_editor_property("mapping_contexts", contexts)
        m.set_editor_property("show_cursor", show_cursor)
        return m

    M = unreal.PlayerMode
    cdo.set_editor_property("mode_input", {
        M.COMBAT: mode([ctx("IMC_Combat")]),
        M.WHEEL: mode([ctx("IMC_Combat"), ctx("IMC_CommandWheel", 10)]),
        M.BUILD: mode([ctx("IMC_Build")], show_cursor=True),
        M.FOCUS: mode([ctx("IMC_TacticalFocus", 10)]),
        M.SPIRIT: mode([ctx("IMC_CommanderSpirit")]),
        M.MODAL: mode([], show_cursor=True),
    })
    cdo.set_editor_property("debug_mapping_context", imc["IMC_Debug"])
    cdo.set_editor_property("debug_toggle_build_action", ia["IA_DebugToggleBuild"])


controller = blueprint("BP_HeroPlayerController", CORE, unreal.HeroPlayerController, setup_controller)

# Foundation-only game mode for L_Boot and test maps; feature maps override it (CMB sandbox mode).
blueprint("BP_BootGameMode", CORE, unreal.GameModeBase,
          lambda cdo: cdo.set_editor_property("player_controller_class", controller.generated_class()))

# T-FND-07: editor-only discovery/validation example; future gameplay types register separately.
TEST_MAPS = f"{ROOT}/Maps/Test"
definition_class = unreal.load_class(None, "/Script/CastleDefender.TestGameDefinition")
definition_factory = unreal.DataAssetFactory()
definition_factory.set_editor_property("data_asset_class", definition_class)
get_or_create("DA_FoundationSmoke", f"{TEST_MAPS}/Definitions", definition_class,
              definition_factory, lambda asset: asset.set_editor_property(
                  "display_name", unreal.Text("Foundation validation example")))

# T-FND-10: use the pinned engine's graph editing API, without editor dependencies in the module.
ft_factory = unreal.BlueprintFactory()
ft_factory.set_editor_property("parent_class", unreal.FunctionalTest)
ft_bp = get_or_create("BP_FT_Smoke", TEST_MAPS, None, ft_factory,
                      lambda bp: (unreal.BlueprintEditorLibrary.add_event_override(bp, "ReceiveStartTest", unreal.IntPoint(0, 0)),
                                  unreal.BlueprintEditorLibrary.compile_blueprint(bp)))


def wire_smoke_test(bp):
    editor = unreal.BlueprintGraphEditor.get_graph_editor(
        unreal.BlueprintEditorLibrary.find_event_graph(bp))
    start = editor.find_event_node("ReceiveStartTest")
    if start.find_then_pin().list_connected_pins():
        return  # Preserve an already wired graph, including user edits.
    if any(p.list_connected_pins() for n in editor.list_all_nodes() for p in n.list_all_pins()):
        raise RuntimeError("Smoke graph has custom wiring; refusing to overwrite it")

    def call(path, x, y=0):
        node = editor.add_call_function_node(path)
        if node is None:
            raise RuntimeError(f"Cannot create {path}")
        node.set_node_pos(unreal.IntPoint(x, y))
        return node

    def link(output, input_pin):
        if not output.try_create_connection(input_pin):
            raise RuntimeError("Smoke graph pin connection failed")

    def value(pin, literal):
        if not pin.set_pin_value(literal):
            raise RuntimeError(f"Invalid smoke pin value: {literal}")

    spawn = editor.create_node_from_name("Game|SpawnActorfromClass", unreal.Vector2D(240, 0), [])
    value(spawn.find_input_pin("Class"), "/Script/CastleDefender.TestDummy")
    value(spawn.find_input_pin("CollisionHandlingOverride"), "AlwaysSpawn")
    transform = call("/Script/Engine.Actor.GetTransform", 0, 240)
    link(transform.find_result_pin(), spawn.find_input_pin("SpawnTransform"))
    link(start.find_then_pin(), spawn.find_execute_pin())
    valid = call("/Script/Engine.KismetSystemLibrary.IsValid", 460, 240)
    link(spawn.find_result_pin(), valid.find_input_pin("Object"))
    guard = editor.add_branch_node()
    guard.set_node_pos(unreal.IntPoint(500, 0))
    link(valid.find_result_pin(), guard.find_condition_pin())
    link(spawn.find_then_pin(), guard.find_execute_pin())
    hit = call("/Script/CastleDefender.TestDummy.ApplyDebugHit", 720)
    value(hit.find_input_pin("Damage"), "1000")
    link(spawn.find_result_pin(), hit.find_self_pin())
    link(guard.find_then_pin(), hit.find_execute_pin())
    health = call("/Script/CastleDefender.TestDummy.GetHealth", 720, 240)
    link(spawn.find_result_pin(), health.find_self_pin())
    dead = call("/Script/CastleDefender.HealthComponent.IsDead", 950, 240)
    link(health.find_result_pin(), dead.find_self_pin())
    check = editor.add_branch_node()
    check.set_node_pos(unreal.IntPoint(1000, 0))
    link(hit.find_then_pin(), check.find_execute_pin())
    link(dead.find_result_pin(), check.find_condition_pin())
    for source, result, message, y in (
            (check.find_then_pin(), "Succeeded", "Dummy died", 0),
            (check.find_else_pin(), "Failed", "Dummy survived", 200),
            (guard.find_else_pin(), "Failed", "Dummy spawn failed", 400)):
        finish = call("/Script/FunctionalTesting.FunctionalTest.FinishTest", 1250, y)
        value(finish.find_input_pin("TestResult"), result)
        value(finish.find_input_pin("Message"), message)
        link(source, finish.find_execute_pin())
    if not unreal.BlueprintEditorLibrary.compile_blueprint(bp) or editor.list_nodes_with_errors() or editor.list_nodes_with_warnings():
        raise RuntimeError("Smoke Blueprint did not compile without warnings")
    assets.save_loaded_asset(bp, only_if_is_dirty=False)
    unreal.log("Wired and saved Foundation smoke test")


wire_smoke_test(ft_bp)

if not assets.does_asset_exist(f"{TEST_MAPS}/FT_Smoke"):
    get_or_create("FT_Smoke", TEST_MAPS, unreal.World, unreal.WorldFactory())
    unreal.EditorLoadingAndSavingUtils.load_map(f"{TEST_MAPS}/FT_Smoke")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    test = actors.spawn_actor_from_class(ft_bp.generated_class(), unreal.Vector(0, 0, 100))
    test.set_actor_label("FT_Smoke_DummyDies")
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"Placed {test.get_actor_label()} in {TEST_MAPS}/FT_Smoke")
