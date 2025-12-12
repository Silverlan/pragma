// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :networking.resource;

static std::vector<std::string> &get_extensions()
{
	static std::vector<std::string> exts;
	if(exts.empty()) {
		exts = {"cache", "txt", "gls", Lua::FILE_EXTENSION_PRECOMPILED};
		for(uint32_t i = 0; i < pragma::math::to_integral(pragma::asset::Type::Count); ++i) {
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
		folders.reserve(folders.size() + pragma::math::to_integral(pragma::asset::Type::Count));
		for(uint32_t i = 0; i < pragma::math::to_integral(pragma::asset::Type::Count); ++i) {
			auto type = static_cast<pragma::asset::Type>(i);
			auto *rootDir = pragma::asset::get_asset_root_directory(type);
			folders.push_back(rootDir);
		}
	}
	return folders;
}

bool pragma::networking::is_valid_resource(std::string res)
{
	auto &extensions = get_extensions();
	if(!ufile::compare_extension(res, extensions))
		return false;
	auto &folders = get_folders();
	if(!ufile::compare_directory(res, folders))
		return false;
	return true;
}
