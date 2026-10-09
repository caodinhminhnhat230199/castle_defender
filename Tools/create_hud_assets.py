"""T-UXF-02: Blueprint-authored HUD shell/vitals. Preserve existing widgets and custom layout."""
import unreal

UI = "/Game/CastleDefender/UI"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

def create_widget(name, parent):
    path = UI + "/" + name
    if assets.does_asset_exist(path): return assets.load_asset(path), False
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    return tools.create_asset(name, UI, None, factory), True

def child(tree, cls, name):
    return unreal.new_object(cls, tree, name)

def canvas_child(canvas, widget, x, y, width, height, anchor=(0, 0), stretch=False):
    slot = canvas.add_child_to_canvas(widget)
    maximum = unreal.Vector2D(1, 1) if stretch else unreal.Vector2D(*anchor)
    slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(*anchor), maximum=maximum))
    slot.set_offsets(unreal.Margin(x, y, width, height))
    return slot

vitals, new = create_widget("WBP_HeroVitals", unreal.HeroVitalsWidget)
if new:
    tree = unreal.find_object(vitals, "WidgetTree")
    root = child(tree, unreal.CanvasPanel, "VitalsCanvas")
    assert unreal.LockOnMarkerWidget.set_editor_root(tree, root)
    for name, label, color, y in (("HealthBar", "HP", unreal.LinearColor(0.85, 0.18, 0.16, 1), 24),
                                 ("StaminaBar", "Stamina", unreal.LinearColor(0.10, 0.75, 0.68, 1), 76)):
        text = child(tree, unreal.TextBlock, name + "Label")
        text.set_text(label)
        canvas_child(root, text, 0, y-24, 280, 24)
        bar = child(tree, unreal.ProgressBar, name)
        bar.set_percent(1.0)
        bar.set_fill_color_and_opacity(color)
        canvas_child(root, bar, 0, y, 280, 20)
    assert unreal.BlueprintEditorLibrary.compile_blueprint(vitals)
    assert assets.save_loaded_asset(vitals, only_if_is_dirty=False)

vignette, new_vignette = create_widget("WBP_DamageVignette", unreal.UserWidget)
if new_vignette:
    tree = unreal.find_object(vignette, "WidgetTree")
    root = child(tree, unreal.CanvasPanel, "VignetteCanvas")
    assert unreal.LockOnMarkerWidget.set_editor_root(tree, root)
    edges = [
        ("TopEdge", (0, 0), (1, 0), unreal.Margin(0, 0, 0, 80)),
        ("BottomEdge", (0, 1), (1, 1), unreal.Margin(0, -80, 0, 80)),
        ("LeftEdge", (0, 0), (0, 1), unreal.Margin(0, 0, 80, 0)),
        ("RightEdge", (1, 0), (1, 1), unreal.Margin(-80, 0, 80, 0)),
    ]
    for name, a_min, a_max, offsets in edges:
        border = child(tree, unreal.Border, name)
        border.set_brush_color(unreal.LinearColor(0.85, 0.05, 0.05, 0.8))
        slot = root.add_child_to_canvas(border)
        slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(*a_min), maximum=unreal.Vector2D(*a_max)))
        slot.set_offsets(offsets)
    assert unreal.BlueprintEditorLibrary.compile_blueprint(vignette)
    assert assets.save_loaded_asset(vignette, only_if_is_dirty=False)

vitals_cdo = unreal.get_default_object(vitals.generated_class())
if vitals_cdo.get_editor_property("damage_vignette_class") is None:
    vitals_cdo.set_editor_property("damage_vignette_class", vignette.generated_class())
    assert unreal.BlueprintEditorLibrary.compile_blueprint(vitals)
    assert assets.save_loaded_asset(vitals, only_if_is_dirty=False)

hud, new = create_widget("WBP_GameHUD", unreal.GameHUDWidget)
if new:
    tree = unreal.find_object(hud, "WidgetTree")
    root = child(tree, unreal.CanvasPanel, "HUDCanvas")
    assert unreal.LockOnMarkerWidget.set_editor_root(tree, root)
    # All fourteen extension points are authored here, not created by gameplay code.
    placements = {
        "Vitals": (32, -144, 300, 112, (0, 1)),
        "Alerts": (-332, 160, 300, 300, (1, 0)),
        "Squads": (-332, -160, 300, 128, (1, 1)),
        "CoreLanes": (-220, 24, 440, 64, (0.5, 0)),
        "RunStatus": (-220, 100, 440, 64, (0.5, 0)),
        "Forecast": (-332, 24, 300, 112, (1, 0)),
        "Focus": (-100, -64, 200, 40, (0.5, 1)),
        "Spirit": (-220, 180, 440, 160, (0.5, 0)),
        "Boss": (-220, 24, 440, 64, (0.5, 0)),
        "Perks": (32, 140, 280, 240, (0, 0)),
        "Prompt": (-140, -220, 280, 48, (0.5, 1)),
        "Debug": (32, 32, 400, 240, (0, 0)),
        "LockOn": (0, 0, 0, 0, (0, 0)),
        "Modal": (0, 0, 0, 0, (0, 0)),
    }
    for name, (x, y, width, height, anchor) in placements.items():
        slot_widget = child(tree, unreal.NamedSlot, name)
        canvas_child(root, slot_widget, x, y, width, height, anchor, name in ("LockOn", "Modal"))
        if name == "Vitals": slot_widget.set_content(child(tree, vitals.generated_class(), "HeroVitals"))
        elif name == "LockOn": slot_widget.set_content(child(tree, unreal.CanvasPanel, "LockOnLayer"))
        else: slot_widget.set_visibility(unreal.SlateVisibility.COLLAPSED)
    assert unreal.BlueprintEditorLibrary.compile_blueprint(hud)
    assert assets.save_loaded_asset(hud, only_if_is_dirty=False)

controller = assets.load_asset("/Game/CastleDefender/Core/BP_HeroPlayerController")
defaults = unreal.get_default_object(controller.generated_class())
if defaults.get_editor_property("game_hud_class") is None:
    defaults.set_editor_property("game_hud_class", hud.generated_class())
    assert unreal.BlueprintEditorLibrary.compile_blueprint(controller)
    assert assets.save_loaded_asset(controller, only_if_is_dirty=False)
unreal.log("HUD_ASSETS complete; existing layouts/tuning preserved")
