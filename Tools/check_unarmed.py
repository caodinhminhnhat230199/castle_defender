import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Mannequins/Anims/Unarmed/ABP_Unarmed")
if abp:
    bel = unreal.BlueprintEditorLibrary
    graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
    if graph:
        nodes = graph.list_all_nodes()
        unreal.log(f"AnimGraph has {len(nodes)} nodes in ABP_Unarmed")
    else:
        unreal.log_error("Could not get AnimGraph")
else:
    unreal.log_error("Could not load ABP_Unarmed")
