import unreal

rtg = unreal.EditorAssetLibrary.load_asset('/Game/CastleDefender/Hero/RTG_UAL_To_Mannequin')
ctrl = unreal.IKRetargeterController.get_controller(rtg)

unreal.log(f"Op methods: {[m for m in dir(ctrl) if 'op' in m.lower()]}")
