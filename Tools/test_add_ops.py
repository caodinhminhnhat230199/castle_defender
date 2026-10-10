import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin")
ctrl = unreal.IKRetargeterController.get_controller(rtg)

unreal.log(f"add_default_ops doc: {ctrl.add_default_ops.__doc__}")
ctrl.add_default_ops()
num_ops = ctrl.get_num_retarget_ops()
unreal.log(f"After add_default_ops, num_ops: {num_ops}")
for i in range(num_ops):
    unreal.log(f"  op {i}: {ctrl.get_op_name(i)}")

ctrl.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
tgt_rig = rtg.get_editor_property("target_ik_rig_asset")
tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)
chains = [c.get_editor_property("chain_name") for c in tgt_ctrl.get_retarget_chains()]
mapped = 0
for c in chains:
    src = ctrl.get_source_chain(c)
    if src != "None": mapped += 1
    unreal.log(f"  {c} -> {src}")
unreal.log(f"Total mapped: {mapped}/{len(chains)}")
