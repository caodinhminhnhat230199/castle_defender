"""Creates the Phase F editor assets that cannot be written as text.

Idempotent: an existing asset is loaded and left unchanged.
Run headless: powershell -File Tools/create_foundation_assets.ps1
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
