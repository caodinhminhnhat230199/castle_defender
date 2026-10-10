import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
if bp:
    cdo = unreal.get_default_object(bp.generated_class())
    mesh = cdo.get_editor_property("mesh")
    anim_mode = mesh.get_editor_property("animation_mode")
    unreal.log(f"Animation Mode: {anim_mode}")
    tick_option = mesh.get_editor_property("visibility_based_anim_tick_option")
    unreal.log(f"Tick Option: {tick_option}")
    skel_mesh = mesh.get_editor_property("skeletal_mesh_asset")
    unreal.log(f"SkeletalMeshAsset: {skel_mesh}")
    if skel_mesh:
        unreal.log(f"Skeleton: {skel_mesh.get_editor_property('skeleton')}")
