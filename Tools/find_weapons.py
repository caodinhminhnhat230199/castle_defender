import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
for p in assets.list_assets("/Game/CastleDefender", recursive=True):
    data = assets.find_asset_data(p)
    name = str(data.asset_name).lower()
    if any(w in name for w in ['sword', 'weapon', 'blade', 'shield', 'mesh', 'sm_']):
        unreal.log(f"Found: {p} ({data.asset_class_path.asset_name})")
