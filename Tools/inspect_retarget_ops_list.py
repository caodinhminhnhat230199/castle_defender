import unreal

rtg = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin')
ctrl = unreal.IKRetargeterController.get_controller(rtg)

unreal.log(f"Number of ops: {ctrl.get_num_retarget_ops()}")
for i in range(ctrl.get_num_retarget_ops()):
    op = ctrl.get_retarget_op_at_index(i)
    unreal.log(f"Op[{i}]: {op} (class={op.get_class().get_name() if op else 'None'}, enabled={ctrl.get_retarget_op_enabled(i)})")
