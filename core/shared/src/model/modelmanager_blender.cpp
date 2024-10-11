/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/modelmanager.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/asset/util_asset.hpp"
#include "pragma/util/util_python.hpp"

pragma::asset::BlenderFormatHandler::BlenderFormatHandler(util::IAssetManager &assetManager, std::string ext) : util::IImportAssetFormatHandler {assetManager}, m_ext {std::move(ext)} {}
bool pragma::asset::BlenderFormatHandler::Import(const std::string &outputPath, std::string &outFilePath)
{
	if(!pragma::python::init_blender()) {
		m_error = "Failed to initialize Python or Blender!";
		return false;
	}
	auto filePath = "models/" + outputPath + '.' + m_ext;
	std::string absPath;
	if(!FileManager::FindAbsolutePath(filePath, absPath)) {
		m_error = "File not found!";
		return false;
	}

	std::string glbMdlPath = "models/" + outputPath + ".glb";
	auto glbPath = "addons/imported/" + glbMdlPath;
	auto absGlbPath = util::Path::CreatePath(util::get_program_path()) + util::Path::CreateFile(glbPath);
	std::vector<const char *> argv {absPath.c_str()};
	if(!pragma::python::exec("modules/blender/scripts/format_importers/" + m_ext + ".py", argv.size(), argv.data())) {
		auto errMsg = pragma::python::get_last_error();
		m_error = "Failed to import asset into Blender instance: ";
		if(errMsg)
			*m_error += *errMsg;
		else
			*m_error += "Unknown error";
		return false;
	}
	argv = {absGlbPath.GetString().c_str()};
	if(!pragma::python::exec("modules/blender/scripts/export_scene_as_glb.py", argv.size(), argv.data())) {
		auto errMsg = pragma::python::get_last_error();
		m_error = "Failed to export scene as glTF/glb: ";
		if(errMsg)
			*m_error += *errMsg;
		else
			*m_error += "Unknown error";
		return false;
	}
	filemanager::update_file_index_cache(absGlbPath.GetString(), true);

	// Asset has been converted to glb, we can now redirect it to the gltf format handler
	auto res = static_cast<util::FileAssetManager &>(GetAssetManager()).Import("models/" +outputPath + ".glb");
	filemanager::remove_file(glbPath); // Don't need the glb anymore
	outFilePath = outputPath;
	return res;
}
