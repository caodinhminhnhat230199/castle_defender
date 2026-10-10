import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary
for g in bel.list_graphs(abp):
    if g.get_name() in ["Idle", "Jump", "Fall Loop", "Land"]:
        editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
        nodes = editor.list_all_nodes()
        unreal.log(f"Graph: {g.get_name()}")
        for n in nodes:
            unreal.log(f"  Node: {n.get_name()} ({n.get_class().get_name()})")
            for prop in ["sequence", "anim_sequence", "animation", "node"]:
                try:
                    val = n.get_editor_property(prop)
                    unreal.log(f"    prop {prop}: {val}")
                except:
                    pass
