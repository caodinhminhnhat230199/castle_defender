import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary

graph = bel.find_graph(abp, "AnimGraph")
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
for n in editor.list_all_nodes():
    if "ControlRig" in n.get_class().get_name():
        node_struct = n.get_editor_property("node")
        unreal.log(f"node_struct: {node_struct}")
        try:
            cr_class = node_struct.get_editor_property("control_rig_class")
            unreal.log(f"Control Rig Class: {cr_class}")
        except Exception as e:
            unreal.log(f"Err cr_class: {e}")
