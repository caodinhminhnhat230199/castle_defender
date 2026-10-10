import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
textures = [
    ("MagicSword_Albedo.png", False),
    ("MagicSword_Normal.png", True),
    ("MagicSword_MetallicSmoothness.png", False),
    ("MagicSword_Emission.png", False),
    ("MagicSword_AO.png", False),
]

tasks = []
for tex_file, is_normal in textures:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", f"C:/Users/ADMIN/Downloads/magic-sword/textures/{tex_file}")
    task.set_editor_property("destination_path", "/Game/CastleDefender/Placeholder/Weapons/Textures")
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    tasks.append(task)

asset_tools.import_asset_tasks(tasks)

for t in tasks:
    imported = t.get_editor_property("imported_object_paths")
    unreal.log(f"Imported: {imported}")
    if is_normal and imported:
        tex = unreal.EditorAssetLibrary.load_asset(imported[0])
        if tex and "Normal" in str(tex.get_name()):
            tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
            tex.set_editor_property("s_rgb", False)
            unreal.EditorAssetLibrary.save_loaded_asset(tex)
