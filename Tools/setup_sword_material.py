import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mat_lib = unreal.MaterialEditingLibrary

mat_path = "/Game/CastleDefender/Placeholder/Weapons"
mat_name = "M_MagicSword"

mat = unreal.EditorAssetLibrary.load_asset(f"{mat_path}/{mat_name}")
if not mat:
    mat = asset_tools.create_asset(mat_name, mat_path, unreal.Material, unreal.MaterialFactoryNew())

# Load textures
t_albedo = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/Textures/MagicSword_Albedo")
t_normal = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/Textures/MagicSword_Normal")
t_metal = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/Textures/MagicSword_MetallicSmoothness")
t_emiss = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/Textures/MagicSword_Emission")
t_ao = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/Textures/MagicSword_AO")

# Setup Base Color
node_albedo = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, -200)
node_albedo.set_editor_property("texture", t_albedo)
mat_lib.connect_material_property(node_albedo, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)

# Setup Normal
if t_normal:
    node_normal = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 200)
    node_normal.set_editor_property("texture", t_normal)
    node_normal.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    mat_lib.connect_material_property(node_normal, "RGB", unreal.MaterialProperty.MP_NORMAL)

# Setup Metallic & Roughness
if t_metal:
    node_metal = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -600, 0)
    node_metal.set_editor_property("texture", t_metal)
    node_metal.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
    mat_lib.connect_material_property(node_metal, "R", unreal.MaterialProperty.MP_METALLIC)
    
    # Roughness = 1 - Alpha (or 1 - Green depending on map)
    one_minus = mat_lib.create_material_expression(mat, unreal.MaterialExpressionOneMinus, -300, 50)
    mat_lib.connect_material_expressions(node_metal, "A", one_minus, "")
    mat_lib.connect_material_property(one_minus, "", unreal.MaterialProperty.MP_ROUGHNESS)

# Setup Emissive
if t_emiss:
    node_emiss = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 400)
    node_emiss.set_editor_property("texture", t_emiss)
    mat_lib.connect_material_property(node_emiss, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

# Setup AO
if t_ao:
    node_ao = mat_lib.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 600)
    node_ao.set_editor_property("texture", t_ao)
    node_ao.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
    mat_lib.connect_material_property(node_ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)

mat_lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.log("Configured and compiled M_MagicSword")

# Assign to SM_MagicSword
sm = unreal.EditorAssetLibrary.load_asset("/Game/CastleDefender/Placeholder/Weapons/SM_MagicSword")
if sm:
    sm.set_material(0, mat)
    unreal.EditorAssetLibrary.save_loaded_asset(sm)
    unreal.log("Assigned M_MagicSword to SM_MagicSword")
