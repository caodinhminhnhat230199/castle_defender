import unreal

a1 = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Assets/UAL2_Standard/SkeletalMeshes/UAL2_StandardSword_Regular_A")
a2 = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Assets/UAL_Standard/AL_StandardRig_Sword_Idle")

if a1:
    unreal.log(f"UAL2 Skeleton: {a1.get_editor_property('skeleton')}")
else:
    unreal.log_error("No UAL2 asset found")

if a2:
    unreal.log(f"AL Skeleton: {a2.get_editor_property('skeleton')}")
else:
    unreal.log_error("No AL asset found")
