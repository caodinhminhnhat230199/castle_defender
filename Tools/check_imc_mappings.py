import unreal

imc = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Core/Input/IMC_Combat")
if imc:
    mappings = imc.get_editor_property("default_key_mappings")
    m_list = mappings.get_editor_property("mappings")
    unreal.log(f"IMC_Combat has {len(m_list)} default_key_mappings:")
    for m in m_list:
        act = m.get_editor_property("action")
        key = m.get_editor_property("key")
        key_name = key.get_editor_property("key_name")
        unreal.log(f"  Action: {act.get_name() if act else 'None'} -> Key: {key_name}")
else:
    unreal.log_error("No IMC_Combat")
