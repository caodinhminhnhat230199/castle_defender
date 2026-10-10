import unreal

da = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")
if da:
    light = da.get_editor_property("light_chain")
    for i, a in enumerate(light):
        unreal.log(f"Light[{i}] StartSocket: '{a.get_editor_property('trace_start_socket')}', EndSocket: '{a.get_editor_property('trace_end_socket')}'")
    heavy = da.get_editor_property("heavy")
    unreal.log(f"Heavy StartSocket: '{heavy.get_editor_property('trace_start_socket')}', EndSocket: '{heavy.get_editor_property('trace_end_socket')}'")
