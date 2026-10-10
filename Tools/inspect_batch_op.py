import unreal

for name in dir(unreal.IKRetargetBatchOperation):
    if not name.startswith("_"):
        unreal.log(f"IKRetargetBatchOperation.{name}: {getattr(unreal.IKRetargetBatchOperation, name).__doc__}")
