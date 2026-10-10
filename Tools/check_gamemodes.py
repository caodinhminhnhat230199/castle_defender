import unreal

for gm_path in ["/Game/CastleDefender/Core/BP_BootGameMode", "/Game/CastleDefender/Core/BP_SandboxGameMode"]:
    gm = unreal.EditorAssetLibrary.load_asset(gm_path)
    if gm:
        cdo = unreal.get_default_object(gm.generated_class())
        pawn_cls = cdo.get_editor_property("default_pawn_class")
        pc_cls = cdo.get_editor_property("player_controller_class")
        unreal.log(f"{gm_path}: DefaultPawn={pawn_cls}, PlayerController={pc_cls}")
    else:
        unreal.log_error(f"Cannot load {gm_path}")
