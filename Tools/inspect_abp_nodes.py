import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary

graph = bel.find_graph(abp, "AnimGraph")
if graph:
    editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
    nodes = editor.list_all_nodes()
    unreal.log(f"AnimGraph nodes count: {len(nodes)}")
    for n in nodes:
        unreal.log(f"Node: {n.get_name()} ({n.get_class().get_name()}) title='{n.get_node_title()}'")
        for p in ["control_rig_class", "alpha", "blend_profile"]:
            try:
                unreal.log(f"   {p}: {n.get_editor_property(p)}")
            except:
                pass
