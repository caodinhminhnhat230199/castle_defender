import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary

graph = bel.find_graph(abp, "AnimGraph")
editor = unreal.BlueprintGraphEditor.get_graph_editor(graph)
for n in editor.list_all_nodes():
    if "ControlRig" in n.get_class().get_name():
        unreal.log(f"ControlRig Node: {n.get_name()}")
        for p in dir(n):
            if "rig" in p.lower() or "class" in p.lower():
                try:
                    unreal.log(f"   {p}: {getattr(n, p)}")
                except:
                    pass
        try:
            val = n.get_editor_property("control_rig_class")
            unreal.log(f"   control_rig_class: {val}")
        except Exception as e:
            unreal.log(f"   err: {e}")
