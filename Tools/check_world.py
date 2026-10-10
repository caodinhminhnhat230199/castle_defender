import unreal

subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
subsystem.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
world = subsystem.get_current_world()
settings = world.get_world_settings()
gm_override = settings.get_editor_property("game_mode_override")
unreal.log(f"L_CombatSandbox GameMode Override: {gm_override}")
if gm_override:
    cdo = unreal.get_default_object(gm_override)
    unreal.log(f"Override DefaultPawn: {cdo.get_editor_property('default_pawn_class')}")
