import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary
pins_lib = unreal.BlueprintGraphPinLibrary

graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
nodes = graph.list_all_nodes()

unreal.log("=== ANIM GRAPH PIN CONNECTIONS ===")
for n in nodes:
    n_name = n.get_name()
    cls_name = n.get_class().get_name()
    for pin in n.get_editor_property("pins"):
        pin_name = pins_lib.get_pin_name(pin)
        pin_dir = pin.get_editor_property("direction") # 0 = Input, 1 = Output
        connected = pins_lib.list_connected_pins(pin)
        if len(connected) > 0 and pin_dir == unreal.EdGraphPinDirection.EGPD_OUTPUT:
            for c in connected:
                target_node = c.get_owning_node().get_name()
                target_pin = pins_lib.get_pin_name(c)
                unreal.log(f"{n_name}.{pin_name} -> {target_node}.{target_pin}")
        elif len(connected) == 0 and pin_dir == unreal.EdGraphPinDirection.EGPD_INPUT:
            # log unconnected inputs
            pass
