import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
UAL2_DIR = "/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes"

data = assets.find_asset_data(f"{UAL2_DIR}/UAL2_StandardSword_Regular_A")
unreal.log(f"data: {data.is_valid()}, package: {data.package_name}, asset_name: {data.asset_name}")
