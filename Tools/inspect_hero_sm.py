import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
hero = assets.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
cdo = unreal.get_default_object(hero.generated_class())
for comp in cdo.get_components_by_class(unreal.StaticMeshComponent):
    sm = comp.get_editor_property("static_mesh")
    unreal.log(f"Comp: {comp.get_name()}, SM: {sm.get_name() if sm else 'None'}, AttachSocket: {comp.get_attach_socket_name()}")
