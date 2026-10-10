import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
rtg = assets.load_asset("/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin")
ctrl = unreal.IKRetargeterController.get_controller(rtg)

tgt_rig = assets.load_asset("/Game/CastleDefender/Hero/IK_Mannequin")
rig_ctrl = unreal.IKRigController.get_controller(tgt_rig)
chains = rig_ctrl.get_retarget_chains()

for c in chains:
    name = c.get_editor_property("chain_name")
    src_chain = ctrl.get_source_chain(name)
    unreal.log(f"UAL: Target '{name}' -> Source '{src_chain}'")
