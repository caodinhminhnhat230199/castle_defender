import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

src_rig = assets.load_asset("/Game/CastleDefender/Assets/IK_UAL2_Standard")
tgt_rig = assets.load_asset("/Game/CastleDefender/Hero/IK_Mannequin")

src_ctrl = unreal.IKRigController.get_controller(src_rig)
tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)

src_chains = [c.get_editor_property("chain_name") for c in src_ctrl.get_retarget_chains()]
tgt_chains = [c.get_editor_property("chain_name") for c in tgt_ctrl.get_retarget_chains()]

unreal.log(f"src_chains ({len(src_chains)}): {src_chains}")
unreal.log(f"tgt_chains ({len(tgt_chains)}): {tgt_chains}")
