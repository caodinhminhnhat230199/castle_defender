import unreal

sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")
if sm:
    unreal.log(f"StaticMesh sockets: {[s.socket_name for s in sm.sockets] if hasattr(sm, 'sockets') else 'no sockets attr'}")
    # How to create socket on StaticMesh?
    # unreal.StaticMeshSocket
    unreal.log(f"StaticMesh methods: {[m for m in dir(sm) if 'socket' in m.lower()]}")
