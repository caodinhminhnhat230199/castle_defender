import unreal

rtg = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin')
ctrl = unreal.IKRetargeterController.get_controller(rtg)

unreal.log("=== OPS IN RTG ===")
# List retarget ops
# How to get ops?
try:
    ops = ctrl.get_retarget_ops()
    for op in ops:
        unreal.log(f"Op: {op}")
except Exception as e:
    unreal.log(f"Error get_retarget_ops: {e}")

# Check chain mapping
unreal.log("=== CHAIN MAPPING ===")
for chain_name in ctrl.get_target_chain_names():
    source_chain = ctrl.get_source_chain_name(chain_name)
    unreal.log(f"Target '{chain_name}' -> Source '{source_chain}'")

# Check source IK Rig and target IK Rig chains
src_rig = ctrl.get_ik_rig(unreal.RetargetSourceOrTarget.SOURCE)
tgt_rig = ctrl.get_ik_rig(unreal.RetargetSourceOrTarget.TARGET)

src_ctrl = unreal.IKRigController.get_controller(src_rig)
tgt_ctrl = unreal.IKRigController.get_controller(tgt_rig)

unreal.log("=== SOURCE IK RIG ROOT / RETARGET ROOT ===")
unreal.log(f"Source Retarget Root: {src_ctrl.get_retarget_root()}")
unreal.log(f"Target Retarget Root: {tgt_ctrl.get_retarget_root()}")
