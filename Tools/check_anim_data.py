import unreal

seq = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/Retargeted/AL_StandardRig_Sword_Idle")
if seq:
    num_frames = seq.get_editor_property("number_of_sampled_keys")
    unreal.log(f"AL_StandardRig_Sword_Idle number_of_sampled_keys: {num_frames}")
    num_curves = len(seq.get_editor_property("curve_data").get_editor_property("float_curves"))
    unreal.log(f"Number of float curves: {num_curves}")
