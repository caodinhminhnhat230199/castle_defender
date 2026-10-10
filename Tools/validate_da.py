import unreal

da = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")
if not da:
    unreal.log_error("Could not load DA_HeroClass_Warlord")
else:
    unreal.log(f"DA_HeroClass_Warlord loaded: {da}")
    for i in range(3):
        res, err = da.validate_light_attack(i)
        unreal.log(f"ValidateLightAttack({i}): {res}, Error: '{err}'")
    res, err = da.validate_heavy_attack()
    unreal.log(f"ValidateHeavyAttack(): {res}, Error: '{err}'")
    for d in [unreal.HeroDodgeDirection.FORWARD, unreal.HeroDodgeDirection.BACKWARD, unreal.HeroDodgeDirection.LEFT, unreal.HeroDodgeDirection.RIGHT]:
        res, err = da.validate_dodge(d)
        unreal.log(f"ValidateDodge({d}): {res}, Error: '{err}'")
