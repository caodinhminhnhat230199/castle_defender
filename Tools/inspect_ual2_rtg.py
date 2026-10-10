import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin")
ctrl = unreal.IKRetargeterController.get_controller(rtg)

tgt_rig = rtg.get_editor_property("target_ik_rig_asset")
tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)
chains = [c.get_editor_property("chain_name") for c in tgt_ctrl.get_retarget_chains()]

for c in chains:
    src = ctrl.get_source_chain(c)
    unreal.log(f"UAL2 RTG: {c} -> {src}")
