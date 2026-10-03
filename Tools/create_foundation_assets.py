"""Creates the Phase F editor assets that cannot be written as text.

Idempotent: existing assets are left untouched.
Run headless: powershell -File Tools/create_foundation_assets.ps1
Or in the editor: Tools > Execute Python Script.
"""
import unreal

ROOT = "/Game/CastleDefender"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)


def create(name, path, asset_class, factory):
    full = f"{path}/{name}"
    if assets.does_asset_exist(full):
        return None
    asset = asset_tools.create_asset(name, path, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"Failed to create {full}")
    unreal.log(f"Created {full}")
    return asset


def save(asset):
    if asset is not None:
        assets.save_loaded_asset(asset, only_if_is_dirty=False)


# T-FND-01: empty boot map.
save(create("L_Boot", f"{ROOT}/Maps", unreal.World, unreal.WorldFactory()))
