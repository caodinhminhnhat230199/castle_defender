import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for rtg_path in ["/Game/CastleDefender/Hero/RTG_UAL2_To_Mannequin", "/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin"]:
    rtg = assets.load_asset(rtg_path)
    ctrl = unreal.IKRetargeterController.get_controller(rtg)
    ctrl.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
    
    tgt_rig = rtg.get_editor_property("target_ik_rig_asset")
    rig_ctrl = unreal.IKRigController.get_controller(tgt_rig)
    chains = rig_ctrl.get_retarget_chains()
    
    mapped_count = 0
    unreal.log(f"--- {rtg.get_name()} ---")
    for c in chains:
        name = c.get_editor_property("chain_name")
        src_chain = ctrl.get_source_chain(name)
        if src_chain != "None":
            mapped_count += 1
        unreal.log(f"  {name} -> {src_chain}")
    unreal.log(f"Total mapped: {mapped_count}/{len(chains)}")
