import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
if abp:
    bel = unreal.BlueprintEditorLibrary
    graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
    if graph:
        nodes = graph.list_all_nodes()
        unreal.log(f"AnimGraph has {len(nodes)} nodes:")
        for n in nodes:
            unreal.log(f"  {n.get_name()} ({n.get_class().get_name()})")
    else:
        unreal.log_error("Could not get AnimGraph")
else:
    unreal.log_error("Could not load ABP_Warlord")
