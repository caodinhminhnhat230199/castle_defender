import unreal

ar = unreal.AssetRegistryHelpers.get_asset_registry()
opts = unreal.AssetRegistryDependencyOptions(include_soft_package_references=True, include_hard_package_references=True, include_searchable_names=False, include_soft_management_references=False, include_hard_management_references=False)
deps = ar.get_dependencies("/Game/CastleDefender/Hero/ABP_Warlord", opts)
unreal.log("ABP_Warlord Dependencies:")
for d in deps:
    unreal.log(f"  {d}")
