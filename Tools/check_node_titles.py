import unreal
from pathlib import Path

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
bel = unreal.BlueprintEditorLibrary

graph = unreal.BlueprintGraphEditor.get_graph_editor(bel.find_graph(abp, "AnimGraph"))
nodes = graph.list_all_nodes()

lines = []
for n in nodes:
    title = n.get_node_title(unreal.NodeTitleType.FULL_TITLE) if hasattr(n, "get_node_title") else ""
    lines.append(f"{n.get_name()}: Title='{title}', Class={n.get_class().get_name()}")

Path(unreal.Paths.project_saved_dir(), "node_titles.txt").write_text("\n".join(lines), encoding="utf-8")
