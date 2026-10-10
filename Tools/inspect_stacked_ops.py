import unreal

for rtg_name in ["RTG_UAL_To_Mannequin", "RTG_UAL2_To_Mannequin"]:
    rtg = unreal.EditorAssetLibrary.load_asset(f"/Game/CastleDefender/Hero/{rtg_name}")
    ctrl = unreal.IKRetargeterController.get_controller(rtg)
    unreal.log(f"=== {rtg_name} (Num Ops: {ctrl.get_num_retarget_ops()}) ===")
    for i in range(ctrl.get_num_retarget_ops()):
        name = ctrl.get_op_name(i)
        unreal.log(f"   [{i}] {name}")
