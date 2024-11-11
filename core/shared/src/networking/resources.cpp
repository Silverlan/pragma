/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/resources.h"
#include "pragma/asset/util_asset.hpp"
#include <sharedutils/util_file.h>

static std::vector<std::string> &get_extensions()
{
	static std::vector<std::string> exts;
	if(exts.empty()) {
		exts = {"cache", "txt", "gls", Lua::FILE_EXTENSION_PRECOMPILED};
		for(uint32_t i = 0; i < umath::to_integral(pragma::asset::Type::Count); ++i) {
			auto type = static_cast<pragma::asset::Type>(i);
			auto typeExts = pragma::asset::get_supported_extensions(type);
			exts.reserve(exts.size() + typeExts.size());
			for(auto &ext : typeExts)
				exts.push_back(ext);
		}
	}
	return exts;
}

static std::vector<std::string> &get_folders()
{
	static std::vector<std::string> folders;
	if(folders.empty()) {
		folders = {"cache", "shaders", "scripts", "data", Lua::SCRIPT_DIRECTORY};
		folders.reserve(folders.size() + umath::to_integral(pragma::asset::Type::Count));
		for(uint32_t i = 0; i < umath::to_integral(pragma::asset::Type::Count); ++i) {
			auto type = static_cast<pragma::asset::Type>(i);
			auto *rootDir = pragma::asset::get_asset_root_directory(type);
			folders.push_back(rootDir);
		}
	}
	return folders;
}

DLLNETWORK bool IsValidResource(std::string res)
{
	auto &extensions = get_extensions();
	if(!ufile::compare_extension(res, extensions))
		return false;
	auto &folders = get_folders();
	if(!ufile::compare_directory(res, folders))
		return false;
	return true;
}
