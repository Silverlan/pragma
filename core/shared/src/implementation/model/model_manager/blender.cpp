// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model_manager;

pragma::asset::BlenderFormatHandler::BlenderFormatHandler(util::IAssetManager &assetManager, std::string ext) : IImportAssetFormatHandler {assetManager}, m_ext {std::move(ext)} {}
bool pragma::asset::BlenderFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	if(!python::init_blender()) {
		m_error = "Failed to initialize Python or Blender!";
		return false;
	}
	auto filePath = "models/" + outputPath + '.' + m_ext;
	std::string absPath;
	if(!fs::find_absolute_path(filePath, absPath)) {
		m_error = "File not found!";
		return false;
	}

	std::string glbMdlPath = "models/" + outputPath + ".glb";
	auto glbPath = "addons/imported/" + glbMdlPath;
	auto absGlbPath = util::Path::CreatePath(fs::get_program_write_path()) + util::Path::CreateFile(glbPath);
	std::vector<const char *> argv {absPath.c_str()};
	if(!python::exec("modules/blender/scripts/format_importers/" + m_ext + ".py", argv.size(), argv.data())) {
		auto errMsg = python::get_last_error();
		m_error = "Failed to import asset into Blender instance: ";
		if(errMsg)
			*m_error += *errMsg;
		else
			*m_error += "Unknown error";
		return false;
	}
	argv = {absGlbPath.GetString().c_str()};
	if(!python::exec("modules/blender/scripts/export_scene_as_glb.py", argv.size(), argv.data())) {
		auto errMsg = python::get_last_error();
		m_error = "Failed to export scene as glTF/glb: ";
		if(errMsg)
			*m_error += *errMsg;
		else
			*m_error += "Unknown error";
		return false;
	}
	fs::update_file_index_cache(absGlbPath.GetString(), true);

	// Asset has been converted to glb, we can now redirect it to the gltf format handler
	auto res = static_cast<util::FileAssetManager &>(GetAssetManager()).Import("models/" + outputPath + ".glb");
	fs::remove_file(glbPath); // Don't need the glb anymore
	outFilePath = outputPath;
	return res;
}
