import unreal

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
unreal.log(f"SubobjectDataSubsystem methods: {[m for m in dir(subsystem) if not m.startswith('_')]}")
