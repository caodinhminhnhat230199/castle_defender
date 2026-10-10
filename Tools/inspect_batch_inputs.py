import unreal

inputs = unreal.IKRetargetBatchOperationInputs()
for p in dir(inputs):
    if not p.startswith("_"):
        unreal.log(f"inputs field: {p}")
