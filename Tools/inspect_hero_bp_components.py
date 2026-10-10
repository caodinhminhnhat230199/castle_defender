import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
if bp:
    # Check SCS (SimpleConstructionScript) or SubobjectDataSubsystem or CDO
    cdo = unreal.get_default_object(bp.generated_class())
    unreal.log(f"CDO WeaponMesh: {cdo.get_editor_property('weapon_mesh')}")
    wm = cdo.get_editor_property('weapon_mesh')
    if wm:
        unreal.log(f"WeaponMesh StaticMesh: {wm.get_editor_property('static_mesh')}")
        unreal.log(f"WeaponMesh RelativeLocation: {wm.get_editor_property('relative_location')}")
        unreal.log(f"WeaponMesh RelativeRotation: {wm.get_editor_property('relative_rotation')}")
        unreal.log(f"WeaponMesh RelativeScale3D: {wm.get_editor_property('relative_scale3d')}")
        unreal.log(f"WeaponMesh AttachSocket: {wm.get_editor_property('attach_socket_name')}")

    # Inspect Blueprint SubobjectData or SimpleConstructionScript
    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    if subsystem:
        handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
        for h in handles:
            data = subsystem.k2_find_subobject_data_from_handle(h)
            desc = subsystem.get_subobject_description(data)
            unreal.log(f"Subobject handle desc: {desc}")
