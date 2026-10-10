import unreal
from pathlib import Path

out_lines = []

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary
pins_lib = unreal.BlueprintGraphPinLibrary

graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
nodes = graph.list_all_nodes()

for n in nodes:
    cls = n.get_class().get_name()
    out_lines.append(f"Node: {n.get_name()} ({cls})")
    for pin_name in ["Result", "Pose", "Source", "BasePose", "BlendPoses_0", "BlendPose_0", "BlendPose_1", "BlendPose_0"]:
        try:
            in_p = bel.find_input_pin(n, pin_name)
            if in_p:
                conns = pins_lib.list_connected_pins(in_p)
                for c in conns:
                    out_lines.append(f"  [IN] {pin_name} <-- {c.get_owning_node().get_name()}.{pins_lib.get_pin_name(c)}")
        except:
            pass
        try:
            out_p = bel.find_output_pin(n, pin_name)
            if out_p:
                conns = pins_lib.list_connected_pins(out_p)
                for c in conns:
                    out_lines.append(f"  [OUT] {pin_name} --> {c.get_owning_node().get_name()}.{pins_lib.get_pin_name(c)}")
        except:
            pass

Path(unreal.Paths.project_saved_dir(), "trace_pins.txt").write_text("\n".join(out_lines), encoding="utf-8")
unreal.log("Wrote trace_pins.txt successfully")
