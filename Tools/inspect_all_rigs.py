import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

for rig_path in ["/Game/CastleDefender/Assets/IK_UAL2_Standard", "/Game/CastleDefender/Assets/IK_AL_Standard", "/Game/CastleDefender/Hero/IK_Mannequin"]:
    rig = assets.load_asset(rig_path)
    ctrl = unreal.IKRigController.get_controller(rig)
    chains = [c.get_editor_property("chain_name") for c in ctrl.get_retarget_chains()]
    unreal.log(f"{rig.get_name()}: {[str(c) for c in chains]}")
