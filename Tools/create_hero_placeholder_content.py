"""Placeholder hero content from the engine's Third Person template (Epic Mannequin).

Run through Tools/create_hero_assets.py. Idempotent:
- copies the needed Mannequin files from the engine template into /Game/Characters (what "Add Feature Pack" does),
- moves them to /Game/CastleDefender/Placeholder/Mannequins with references fixed up.
"""
import shutil
from pathlib import Path
import unreal

PLACEHOLDER = "/Game/CastleDefender/Placeholder"
MANNEQUIN = f"{PLACEHOLDER}/Mannequins"
TEMPLATE_SUBDIRS = ["Meshes", "Materials", "Textures", "Rigs", "Anims/Unarmed", "Anims/Death", "Anims/Rifle/HitReact"]

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
registry = unreal.AssetRegistryHelpers.get_asset_registry()


def ensure_mannequin():
    if assets.does_directory_exist(MANNEQUIN) and assets.list_assets(MANNEQUIN):
        return
    content = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()))
    staged = content / "Characters" / "Mannequins"
    if not staged.exists():
        source = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.engine_dir())).parent / \
            "Templates/TemplateResources/High/Characters/Content/Mannequins"
        for sub in TEMPLATE_SUBDIRS:
            shutil.copytree(source / sub, staged / sub, dirs_exist_ok=True)
    registry.scan_paths_synchronous(["/Game/Characters"], True)
    if not assets.rename_directory("/Game/Characters/Mannequins", MANNEQUIN):
        raise RuntimeError("Could not move the Mannequin pack into Placeholder/")
    redirectors = [data.get_asset() for data in registry.get_assets_by_path("/Game/Characters", True)
                   if str(data.asset_class_path.asset_name) == "ObjectRedirector"]
    if redirectors:
        asset_tools.fixup_referencers(redirectors)
    assets.delete_directory("/Game/Characters")
    unreal.log(f"Mannequin placeholder pack installed at {MANNEQUIN}")


def anim(path):
    seq = assets.load_asset(f"{MANNEQUIN}/Anims/{path}")
    if not seq:
        raise RuntimeError(f"Missing placeholder animation {path}")
    return seq
