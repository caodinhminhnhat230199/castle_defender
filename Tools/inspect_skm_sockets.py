import unreal

skm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Characters/Warlord/SKM_Warlord")
if skm:
    for sock in ["hand_r", "hand_r_socket", "weapon_r", "hand_l"]:
        s = skm.find_socket(sock)
        unreal.log(f"find_socket({sock}): {s}")
