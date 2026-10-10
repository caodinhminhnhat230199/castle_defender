import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

src_rig = assets.load_asset("/Game/CastleDefender/Assets/IK_UAL2_Standard")
tgt_rig = assets.load_asset("/Game/CastleDefender/Hero/IK_Mannequin")

def inspect_rig(rig):
    if not rig: return
    ctrl = unreal.IKRigController.get_controller(rig)
    chains = ctrl.get_retarget_chains()
    unreal.log(f"Rig {rig.get_name()}: {len(chains)} chains")
    for c in chains:
        name = c.get_editor_property("chain_name")
        start = c.get_editor_property("start_bone").get_editor_property("bone_name")
        end = c.get_editor_property("end_bone").get_editor_property("bone_name")
        unreal.log(f"  Chain '{name}': {start} -> {end}")

inspect_rig(src_rig)
inspect_rig(tgt_rig)
