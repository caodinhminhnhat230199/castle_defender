import unreal

abp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/ABP_Warlord")
graphs = unreal.BlueprintEditorLibrary.list_graphs(abp)
for g in graphs:
    unreal.log(f"Graph: {g.get_name()} ({g.get_class().get_name()})")
