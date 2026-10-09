"""T-CMB-10 editor content. Called by create_hero_assets.py; preserves existing authored content."""
import math
import unreal

ROOT = "/Game/CastleDefender"
HERO = f"{ROOT}/Hero"
INPUT = f"{ROOT}/Core/Input"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()
bel, pins = unreal.BlueprintEditorLibrary, unreal.BlueprintGraphPinLibrary


def setup_lock_on(hero_bp, abp, definition, anim):
    switch_path = f"{INPUT}/IA_LockOnSwitch"
    switch = assets.load_asset(switch_path) if assets.does_asset_exist(switch_path) else None
    if not switch:
        switch = tools.create_asset("IA_LockOnSwitch", INPUT, unreal.InputAction, unreal.InputAction_Factory())
        switch.set_editor_property("value_type", unreal.InputActionValueType.AXIS1D)
        assets.save_loaded_asset(switch)
    imc = assets.load_asset(f"{INPUT}/IMC_Combat")
    data = imc.get_editor_property("default_key_mappings")
    mappings = list(data.get_editor_property("mappings"))
    if not any(row.get_editor_property("action") == switch for row in mappings):
        for key_name, negative in [("MouseScrollUp", False), ("MouseScrollDown", True)]:
            key = unreal.Key()
            key.import_text(key_name)
            row = unreal.EnhancedActionKeyMapping()
            row.set_editor_property("action", switch)
            row.set_editor_property("key", key)
            if negative:
                row.set_editor_property("modifiers", [unreal.new_object(unreal.InputModifierNegate, imc)])
            mappings.append(row)
        data.set_editor_property("mappings", mappings)
        imc.set_editor_property("default_key_mappings", data)
        assets.save_loaded_asset(imc)
    cdo = unreal.get_default_object(hero_bp.generated_class())
    cdo.set_editor_property("lock_on_action", assets.load_asset(f"{INPUT}/IA_LockOn"))
    cdo.set_editor_property("lock_on_switch_action", switch)
    setup_strafe(abp, definition, anim)
    setup_side_dodges(definition, anim)
    setup_marker()


def setup_strafe(abp, definition, anim):
    path = f"{HERO}/BS_Warlord_Strafe"
    if assets.does_asset_exist(path):
        blend = assets.load_asset(path)
    else:
        factory = unreal.BlendSpaceFactoryNew()
        factory.set_editor_property("target_skeleton", anim("Unarmed/MM_Idle").get_editor_property("skeleton"))
        blend = tools.create_asset("BS_Warlord_Strafe", HERO, unreal.BlendSpace, factory)
        movement = definition.get_editor_property("movement")
        jog, sprint = movement.get_editor_property("jog_speed"), movement.get_editor_property("sprint_speed")
        axes = list(blend.get_editor_property("blend_parameters"))
        for axis, name in zip(axes[:2], ["Right Speed", "Forward Speed"]):
            axis.set_editor_property("display_name", name)
            axis.set_editor_property("min", -sprint)
            axis.set_editor_property("max", sprint)
        blend.set_editor_property("blend_parameters", axes)
        samples = []

        def sample(sequence, right, forward, rate=1.0):
            value = unreal.BlendSample()
            value.set_editor_property("animation", sequence)
            value.set_editor_property("sample_value", unreal.Vector(right, forward, 0.0))
            value.set_editor_property("rate_scale", rate)
            samples.append(value)

        sample(anim("Unarmed/MM_Idle"), 0.0, 0.0)
        for suffix, right, forward in [
            ("Fwd", 0, 1), ("Bwd", 0, -1), ("Left", -1, 0), ("Right", 1, 0),
            ("Fwd_Left", -1, 1), ("Fwd_Right", 1, 1), ("Bwd_Left", -1, -1), ("Bwd_Right", 1, -1),
        ]:
            length = math.hypot(right, forward)
            for speed, directory, rate in [(jog / 2, "Walk", 1.0), (jog, "Jog", 1.0), (sprint, "Jog", sprint / jog)]:
                sample(anim(f"Unarmed/{directory}/MF_Unarmed_{directory}_{suffix}"), speed * right / length, speed * forward / length, rate)
        blend.set_editor_property("sample_data", samples)

    # SampleData assignment alone does not build the runtime triangulation in UE 5.8.
    assert unreal.HeroCombatLibrary.finalize_strafe_blend_space(blend)
    assets.save_loaded_asset(blend)

    graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
    if any(n.get_class().get_name() == "AnimGraphNode_BlendSpacePlayer" and
           n.get_editor_property("node").get_editor_property("blend_space") == blend for n in graph.list_all_nodes()):
        return
    layers = [n for n in graph.list_all_nodes() if n.get_class().get_name() == "AnimGraphNode_LayeredBoneBlend"]
    if len(layers) != 1:
        raise RuntimeError("Strafe wiring expects the existing guard layer")
    base = bel.find_input_pin(layers[0], "BasePose")
    sources = pins.list_connected_pins(base)
    if len(sources) != 1:
        raise RuntimeError("Guard base pose must have one source")
    menu = graph.list_available_nodes([])

    def add_matching(predicate, position):
        matches = [name for name in menu if predicate(name)]
        if len(matches) != 1:
            raise RuntimeError(f"Ambiguous graph action: {matches}")
        return graph.create_node_from_name(matches[0], position, [])

    player = add_matching(lambda name: name.endswith("BlendspacePlayer'BS_Warlord_Strafe'"), unreal.Vector2D(-600, 700))
    selector = add_matching(lambda name: "blendposesbybool" in name.replace(" ", "").lower(), unreal.Vector2D(-250, 650))
    locked = graph.add_get_member_variable_node("bIsLockedOn")
    forward = graph.add_get_member_variable_node("StrafeForwardSpeed")
    right = graph.add_get_member_variable_node("StrafeRightSpeed")
    links = [
        (sources[0], bel.find_input_pin(selector, "BlendPose_1")),
        (bel.find_output_pin(player, "Pose"), bel.find_input_pin(selector, "BlendPose_0")),
        (bel.find_output_pin(locked, "bIsLockedOn"), bel.find_input_pin(selector, "bActiveValue")),
        (bel.find_output_pin(right, "StrafeRightSpeed"), bel.find_input_pin(player, "X")),
        (bel.find_output_pin(forward, "StrafeForwardSpeed"), bel.find_input_pin(player, "Y")),
    ]
    for source, target in links:
        if not pins.try_create_connection(source, target):
            raise RuntimeError(f"Could not connect strafe: {pins.get_pin_name(source)} -> {pins.get_pin_name(target)}")
    pins.break_pin_links(base)
    if not pins.try_create_connection(bel.find_output_pin(selector, "Pose"), base):
        raise RuntimeError("Could not connect strafe to guard layer")
    if not bel.compile_blueprint(abp) or graph.list_nodes_with_errors() or graph.list_nodes_with_warnings():
        raise RuntimeError("Strafe graph failed compile validation")
    assets.save_loaded_asset(abp)


def setup_side_dodges(definition, anim):
    # Placeholder side steps: side-jog poses with the existing dash's lateral travel profile.
    # Production clips remain T-CMB-19. Never replace a user-authored montage source.
    dodge, dash = definition.get_editor_property("dodge"), anim("Unarmed/Jump/MM_Dash")
    authored_sides = True
    for suffix, field in [("L", "left_montage"), ("R", "right_montage")]:
        sequence_path = f"{HERO}/A_Warlord_Dodge_{suffix}"
        if assets.does_asset_exist(sequence_path):
            side = assets.load_asset(sequence_path)
        else:
            side = assets.duplicate_asset(anim(f"Unarmed/Jog/MF_Unarmed_Jog_{'Right' if suffix == 'R' else 'Left'}").get_path_name(), sequence_path)
            assert unreal.HeroCombatLibrary.author_side_dodge(side, dash, 0.8, suffix == "R")
            assets.save_loaded_asset(side)
        montage = dodge.get_editor_property(field)
        tracks = montage.get_editor_property("slot_anim_tracks")
        segment = tracks[0].get_editor_property("anim_track").get_editor_property("anim_segments")[0]
        if segment.get_editor_property("anim_reference") == dash:
            assert unreal.HeroCombatLibrary.set_single_segment_montage_source(montage, side, 0.0, side.get_play_length(), montage.get_play_length(), False)
            assets.save_loaded_asset(montage)
        elif segment.get_editor_property("anim_reference") != side:
            authored_sides = False
    if authored_sides:
        dodge.set_editor_property("side_clips_face_input", False)
    definition.set_editor_property("dodge", dodge)
    assets.save_loaded_asset(definition)


def setup_marker():
    path = f"{ROOT}/UI/WBP_LockOnMarker"
    new_marker = not assets.does_asset_exist(path)
    if not new_marker:
        marker = assets.load_asset(path)
    else:
        factory = unreal.WidgetBlueprintFactory()
        factory.set_editor_property("parent_class", unreal.load_class(None, "/Script/CastleDefender.LockOnMarkerWidget"))
        marker = tools.create_asset("WBP_LockOnMarker", f"{ROOT}/UI", None, factory)
        # The editor tree is an inner UObject; UE 5.8 Python does not expose WidgetBlueprint.WidgetTree.
        tree = unreal.find_object(marker, "WidgetTree")
        if not tree:
            raise RuntimeError("Marker's editor WidgetTree was not found")
        image = unreal.new_object(unreal.Image, tree, "Marker")
        assert unreal.LockOnMarkerWidget.set_editor_root(tree, image)
    tree = unreal.find_object(marker, "WidgetTree")
    image = unreal.find_object(tree, "Marker")
    if image:
        brush = image.get_editor_property("brush")
        # Border requires a texture. RoundedBox paints a hollow outline without a resource.
        if (new_marker or brush.get_editor_property("draw_as") == unreal.SlateBrushDrawType.BORDER) and not brush.get_editor_property("resource_object"):
            brush.set_editor_property("draw_as", unreal.SlateBrushDrawType.ROUNDED_BOX)
            brush.set_editor_property("tint_color", unreal.SlateColor(specified_color=unreal.LinearColor(0, 0, 0, 0)))
            outline = brush.get_editor_property("outline_settings")
            outline.set_editor_property("width", 2.0)
            outline.set_editor_property("color", unreal.SlateColor(specified_color=unreal.LinearColor(1.0, 0.85, 0.15, 1.0)))
            outline.set_editor_property("use_brush_transparency", False)
            brush.set_editor_property("outline_settings", outline)
            image.set_editor_property("brush", brush)
            image.set_editor_property("color_and_opacity", unreal.LinearColor(1, 1, 1, 1))
            if not bel.compile_blueprint(marker):
                raise RuntimeError("Marker blueprint failed to compile")
            assets.save_loaded_asset(marker)
    controller = assets.load_asset(f"{ROOT}/Core/BP_HeroPlayerController")
    cdo = unreal.get_default_object(controller.generated_class())
    if not cdo.get_editor_property("lock_on_marker_class"):
        cdo.set_editor_property("lock_on_marker_class", marker.generated_class())
        assert bel.compile_blueprint(controller)
        assets.save_loaded_asset(controller)
