import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin")
ctrl = unreal.IKRetargeterController.get_controller(rtg)

num_ops = ctrl.get_num_retarget_ops()
unreal.log(f"num_ops: {num_ops}")
for i in range(num_ops):
    op_name = ctrl.get_op_name(i)
    unreal.log(f"op {i}: {op_name}")
