import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/CastleDefender/Maps/L_CombatSandbox")
bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(bp.generated_class(), unreal.Vector(0, 0, 100))

if pawn:
    mesh = pawn.get_editor_property("mesh")
    anim_inst = mesh.get_anim_instance()
    unreal.log(f"Spawned pawn mesh: {mesh}, AnimInstance: {anim_inst}")
    da = pawn.get_hero_class_definition()
    light0 = da.get_editor_property("light_chain")[0].get_editor_property("montage")
    heavy = da.get_editor_property("heavy").get_editor_property("montage")
    dodge = da.get_editor_property("dodge").get_editor_property("forward_montage")
    
    unreal.log(f"Testing PlayAnimMontage Light0: {pawn.play_anim_montage(light0)}")
    unreal.log(f"Testing PlayAnimMontage Heavy: {pawn.play_anim_montage(heavy)}")
    unreal.log(f"Testing PlayAnimMontage Dodge: {pawn.play_anim_montage(dodge)}")
    
    unreal.EditorLevelLibrary.destroy_actor(pawn)
else:
    unreal.log_error("Could not spawn pawn")
