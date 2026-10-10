import unreal

bp = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Hero/BP_Hero_Warlord")
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")

# Let's inspect SubobjectDataSubsystem
subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
unreal.log(f"Found {len(handles)} subobject handles")

wm_handle = None
for h in handles:
    data = subsystem.k2_find_subobject_data_from_handle(h)
    desc = subsystem.get_subobject_description(data)
    obj = subsystem.get_object_for_subobject_data(data)
    unreal.log(f"Subobject: {desc}, obj: {obj}")
    if "WeaponMesh" in str(desc):
        wm_handle = h
        break

if wm_handle:
    data = subsystem.k2_find_subobject_data_from_handle(wm_handle)
    wm_obj = subsystem.get_object_for_subobject_data(data)
    unreal.log(f"Found WeaponMesh subobject: {wm_obj}")
    # Modify properties
    wm_obj.set_editor_property("static_mesh", sm)
    wm_obj.set_editor_property("relative_scale3d", unreal.Vector(0.5, 0.5, 0.5))
    
    # Let's check rotation & location
    # Default alignment:
    # Blade is +Y. In hand_r:
    # If rot is (pitch=-90, yaw=-90, roll=0) or (pitch=0, yaw=-90, roll=0)
    # Let's test standard hand_r sword rotation:
    # In UE mannequin: Pitch=0, Yaw=-90, Roll=-90 aligns sword +Y along thumb/fist pointing forward/up
    wm_obj.set_editor_property("relative_rotation", unreal.Rotator(0.0, -90.0, -90.0))
    wm_obj.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, 0.0))

# Compile Blueprint
unreal.KismetEditorUtilities.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_loaded_asset(bp)
unreal.log("Blueprint compiled and saved successfully.")
