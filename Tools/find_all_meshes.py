import unreal
import os

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

unreal.log("=== CHECKING ALL STATIC MESHES IN /Game ===")
all_assets = assets.list_assets("/Game", recursive=True)
sm_list = []
for p in all_assets:
    ad = assets.find_asset_data(p)
    cls_name = str(ad.asset_class_path.asset_name)
    if cls_name in ["StaticMesh", "SkeletalMesh"]:
        sm_list.append((p, cls_name))

for p, cls_name in sm_list:
    unreal.log(f"[{cls_name}] {p}")

if not sm_list:
    unreal.log("No StaticMesh or SkeletalMesh found in /Game")
