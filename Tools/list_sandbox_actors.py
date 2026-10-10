import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
if world:
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for a in actors:
        unreal.log(f"Actor in map: {a.get_name()} ({a.get_class().get_name()})")
