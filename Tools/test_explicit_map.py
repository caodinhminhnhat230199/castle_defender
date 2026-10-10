import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for rtg_path in ["/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin", "/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin"]:
    rtg = assets.load_asset(rtg_path)
    ctrl = unreal.IKRetargeterController.get_controller(rtg)
    
    src_rig = rtg.get_editor_property("source_ik_rig_asset")
    tgt_rig = rtg.get_editor_property("target_ik_rig_asset")
    
    src_ctrl = unreal.IKRigController.get_controller(src_rig)
    src_chains = {c.get_editor_property("chain_name"): c for c in src_ctrl.get_retarget_chains()}
    
    tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)
    tgt_chains = [c.get_editor_property("chain_name") for c in tgt_ctrl.get_retarget_chains()]
    
    mapped = 0
    for t_name in tgt_chains:
        if t_name in src_chains:
            res = ctrl.set_source_chain(t_name, t_name)
            unreal.log(f"Mapping {t_name} -> {t_name}: {res}")
            mapped += 1
            
    assets.save_loaded_asset(rtg, only_if_is_dirty=False)
    unreal.log(f"Saved {rtg.get_name()} with {mapped} mapped chains!")

