import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
for p in assets.list_assets("/Game/CastleDefender", recursive=True):
    if "idle" in p.lower():
        data = assets.find_asset_data(p)
        unreal.log(f"Idle Asset: {p} ({data.asset_class_path.asset_name})")
