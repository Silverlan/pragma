// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "fsys/filesystem.h"

#include "memory"

#include "pragma/logging.hpp"

#include <sharedutils/util.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_markup_file.hpp>

module pragma.shared;

import :util.steam;

std::vector<util::Path> util::steam::find_steam_root_paths()
{
	auto installationPath = find_steam_installation_path();
	if(!installationPath)
		return {};
	std::vector<util::Path> paths;
	paths.push_back(util::get_normalized_path(*installationPath));

	std::vector<std::string> additionalSteamPaths {};
	get_external_steam_locations(*installationPath, additionalSteamPaths);
	paths.reserve(paths.size() + additionalSteamPaths.size());
	for(auto &path : additionalSteamPaths)
		paths.push_back(util::get_normalized_path(path));
	return paths;
}

std::optional<std::string> util::steam::find_steam_installation_path()
{
	std::string rootSteamPath;
#ifdef _WIN32
	if(util::get_registry_key_value(util::HKey::CurrentUser, "SOFTWARE\\Valve\\Steam", "SteamPath", rootSteamPath) == false)
		return {};
#else
	auto *pHomePath = getenv("HOME");
	if(pHomePath != nullptr)
		rootSteamPath = pHomePath;
	else
		rootSteamPath = "";
	rootSteamPath += "/.steam/root";
	char rootSteamPathLink[PATH_MAX];
	auto *result = realpath(rootSteamPath.c_str(), rootSteamPathLink);
	if(result != nullptr) {
		rootSteamPath = rootSteamPathLink;
	}
	else {
		auto snapPath = util::DirPath(std::string {pHomePath}) + "/snap/steam/common/.local/share/Steam/";
		if(filemanager::is_system_dir(snapPath.GetString()) == true)
			rootSteamPath = snapPath.GetString();
		else
			spdlog::info("Cannot find steam installation!");
		return {};
	}
	//rootSteamPath += "/.local/share/Steam";
#endif
	return rootSteamPath;
}

bool util::steam::get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations)
{
	auto f = FileManager::OpenSystemFile((steamRootPath + "/steamapps/libraryfolders.vdf").c_str(), "r");
	if(f == nullptr)
		return false;
	auto lenContents = f->GetSize();

	DataStream dsContents {static_cast<uint32_t>(lenContents)};
	f->Read(dsContents->GetData(), lenContents);

	util::MarkupFile mf {dsContents};
	auto vdfData = std::make_shared<util::steam::vdf::Data>();
	auto r = util::steam::vdf::read_vdf_block(mf, vdfData->dataBlock);
	if(r != util::MarkupFile::ResultCode::Ok)
		return false;
	auto it = vdfData->dataBlock.children.find("libraryfolders");
	if(it == vdfData->dataBlock.children.end())
		return false;
	auto &libraryFolders = it->second;
	auto fAddPath = [&outExtLocations](std::string path) {
		ustring::replace(path, "\\\\", "/");
		if(path.empty() == false && path.back() == '/')
			path.pop_back();
		outExtLocations.push_back(path);
	};
	for(uint8_t i = 1; i <= 8; ++i) // 8 is supposedly the max number of external locations you can specify in steam
	{
		auto itKv = libraryFolders.keyValues.find(std::to_string(i));
		if(itKv != libraryFolders.keyValues.end())
			fAddPath(itKv->second);
		else {
			// Newer versions of Steam use a different format
			auto itChild = libraryFolders.children.find(std::to_string(i));
			if(itChild != libraryFolders.children.end()) {
				auto itMounted = itChild->second.keyValues.find("mounted");
				if(itMounted != itChild->second.keyValues.end() && itMounted->second == "0")
					continue;
				auto itPath = itChild->second.keyValues.find("path");
				if(itPath != itChild->second.keyValues.end())
					fAddPath(itPath->second);
			}
		}
	}
	return true;
}
