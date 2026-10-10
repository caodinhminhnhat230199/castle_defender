import unreal

pc_bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Core/BP_HeroPlayerController")
if pc_bp:
    cdo = unreal.get_default_object(pc_bp.generated_class())
    mode_input = cdo.get_editor_property("mode_input")
    unreal.log(f"ModeInput keys: {list(mode_input.keys()) if hasattr(mode_input, 'keys') else mode_input}")
    for k, v in mode_input.items():
        ctxs = v.get_editor_property("mapping_contexts")
        names = [c.get_editor_property("mapping_context").get_name() if c.get_editor_property("mapping_context") else "None" for c in ctxs]
        unreal.log(f"Mode {k}: {names}")
else:
    unreal.log_error("No BP_HeroPlayerController")
