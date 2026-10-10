import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
if world:
    ws = world.get_world_settings()
    gm = ws.get_editor_property("game_mode_override")
    unreal.log(f"L_CombatSandbox GameModeOverride: {gm}")
else:
    unreal.log_error("Could not load map")
