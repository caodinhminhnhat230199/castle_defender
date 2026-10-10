import unreal
seq = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle")
if seq:
    skel = seq.get_editor_property("skeleton")
    unreal.log(f"Sequence Skeleton: {skel}")
else:
    unreal.log_error("Sequence not found")

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
if abp:
    skel = abp.get_editor_property("target_skeleton")
    unreal.log(f"ABP Skeleton: {skel}")

mesh = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
if mesh:
    # Need to load CDO
    cdo = unreal.get_default_object(mesh.generated_class())
    mesh_comp = cdo.get_editor_property("mesh")
    skm = mesh_comp.get_editor_property("skeletal_mesh")
    if skm:
        unreal.log(f"Character Mesh: {skm.get_name()}")
        unreal.log(f"Character Mesh Skeleton: {skm.get_editor_property('skeleton')}")
    else:
        unreal.log("No skeletal mesh set on BP_Hero_Warlord!")
