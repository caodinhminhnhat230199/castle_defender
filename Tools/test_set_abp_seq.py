import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

test_seq = assets.load_asset("/Game/CastleDefender/Hero/Animations/UAL2_StandardSword_Regular_A")

for g in bel.list_graphs(abp):
    if g.get_name() == "Idle":
        editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
        nodes = editor.list_all_nodes()
        for n in nodes:
            if n.get_class().get_name() == "AnimGraphNode_SequencePlayer":
                node_struct = n.get_editor_property("node")
                node_struct.set_editor_property("sequence", test_seq)
                n.set_editor_property("node", node_struct)
                unreal.log("Updated sequence on Idle sequence player")

bel.compile_blueprint(abp)
assets.save_loaded_asset(abp, only_if_is_dirty=False)

# Reload and verify
abp_reloaded = assets.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
for g in bel.list_graphs(abp_reloaded):
    if g.get_name() == "Idle":
        editor = unreal.BlueprintGraphEditor.get_graph_editor(g)
        nodes = editor.list_all_nodes()
        for n in nodes:
            if n.get_class().get_name() == "AnimGraphNode_SequencePlayer":
                node_struct = n.get_editor_property("node")
                seq = node_struct.get_editor_property("sequence")
                unreal.log(f"Verified Idle sequence: {seq.get_path_name() if seq else 'None'}")
