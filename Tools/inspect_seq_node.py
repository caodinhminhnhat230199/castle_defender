import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary
for g in bel.list_graphs(abp):
    if g.get_name() == "Idle":
        editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
        nodes = editor.list_all_nodes()
        for n in nodes:
            if n.get_class().get_name() == "AnimGraphNode_SequencePlayer":
                unreal.log(f"Node class: {n.get_class().get_name()}")
                node_struct = n.get_editor_property("node")
                unreal.log(f"node struct dir: {dir(node_struct)}")
                unreal.log(f"node export text: {node_struct.export_text()}")
                for p in ["sequence", "anim_sequence", "play_rate"]:
                    try:
                        val = node_struct.get_editor_property(p)
                        unreal.log(f"struct prop {p}: {val}")
                    except Exception as e:
                        unreal.log(f"struct prop {p} err: {e}")
