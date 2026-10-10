import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin")
mesh_src = assets.load_asset("/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_Standard")
mesh_tgt = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

UAL2_DIR = "/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes"
data = assets.find_asset_data(f"{UAL2_DIR}/UAL2_StandardSword_Regular_A")

inputs = unreal.IKRetargetBatchOperationInputs()
inputs.set_editor_property("assets_to_retarget", [data])
inputs.set_editor_property("ik_retarget_asset", rtg)
inputs.set_editor_property("source_mesh", mesh_src)
inputs.set_editor_property("target_mesh", mesh_tgt)
inputs.set_editor_property("target_path", "/Game/CastleDefender/Hero/Retargeted")

res = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
unreal.log(f"res count: {len(res)}")
for r in res:
    obj = r.get_asset()
    if obj:
        saved = assets.save_loaded_asset(obj, only_if_is_dirty=False)
        unreal.log(f"  saved {obj.get_name()}: {saved}")
