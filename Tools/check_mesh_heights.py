import unreal

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
src_anim = assets.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle")

# Check source mesh AL_Standard bounds and height
src_mesh = assets.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_Standard")
tgt_mesh = assets.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")

unreal.log(f"src_mesh height: {src_mesh.get_bounds().box_extent.z * 2}")
unreal.log(f"tgt_mesh height: {tgt_mesh.get_bounds().box_extent.z * 2}")
