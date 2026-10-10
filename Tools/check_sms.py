import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary

graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
nodes = graph.list_all_nodes()

for n in nodes:
    if "StateMachine" in n.get_class().get_name():
        sm_node = n
        inner_graph = sm_node.get_editor_property("bound_graph")
        unreal.log(f"SM Node: {n.get_name()}, BoundGraph: {inner_graph.get_name() if inner_graph else 'None'}")
