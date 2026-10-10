import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
if abp:
    bel = unreal.BlueprintEditorLibrary
    graphs = abp.get_editor_property("ubergraph_pages")
    if graphs:
        unreal.log("UberGraphs:")
        for g in graphs:
            unreal.log(f"  {g.get_name()}")
            for n in g.get_editor_property("nodes"):
                unreal.log(f"    {n.get_name()} ({n.get_class().get_name()})")
    else:
        unreal.log_error("No ubergraph_pages (Event Graph)")
else:
    unreal.log_error("Could not load ABP_Warlord")
