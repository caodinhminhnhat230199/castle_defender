import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

task = unreal.AssetImportTask()
task.set_editor_property("filename", "C:/Users/ADMIN/Downloads/magic-sword/source/MagicSword.fbx")
task.set_editor_property("destination_path", "/Game/CastleDefender/Placeholder/Weapons")
task.set_editor_property("destination_name", "SM_MagicSword")
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", True)

# FBX import options
options = unreal.FbxImportUI()
options.set_editor_property("import_mesh", True)
options.set_editor_property("import_as_skeletal", False)
options.set_editor_property("import_materials", True)
options.set_editor_property("import_textures", False)
options.static_mesh_import_data.set_editor_property("combine_meshes", True)
task.set_editor_property("options", options)

asset_tools.import_asset_tasks([task])

unreal.log(f"Import task result imported object: {task.get_editor_property('imported_object_paths')}")
