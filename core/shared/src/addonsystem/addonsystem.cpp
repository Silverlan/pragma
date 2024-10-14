/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/addonsystem/addonsystem.h"
#include "pragma/logging.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_file.h>
#include <fsys/directory_watcher.h>
#ifdef _WIN32
#include <sharedutils/util_link.hpp>
#endif

import pragma.uva;
import pragma.pad;

extern DLLNETWORK Engine *engine;

decltype(AddonSystem::m_addons) AddonSystem::m_addons;
decltype(AddonSystem::m_addonWatcher) AddonSystem::m_addonWatcher = nullptr;

pragma::pad::PADPackage *AddonSystem::LoadPADPackage(const std::string &path)
{
	auto it = std::find_if(m_addons.begin(), m_addons.end(), [&path](const AddonInfo &addon) { return FileManager::ComparePath(addon.GetLocalPath(), path); });
	if(it != m_addons.end())
		return nullptr;
	auto *package = dynamic_cast<pragma::pad::PADPackage *>(FileManager::LoadPackage(path.c_str(), static_cast<fsys::SearchFlags>(FSYS_SEARCH_ADDON)));
	if(package != nullptr)
		m_addons.push_back(AddonInfo(path, package->GetPackageVersion(), package->GetPackageId()));
	return package;
}

static void update_package_paths()
{

	auto *sv = engine->GetServerNetworkState();
	if(sv != nullptr && sv->IsGameActive())
		sv->GetGameState()->UpdatePackagePaths();
	auto *cl = engine->GetClientState();
	if(cl != nullptr && cl->IsGameActive())
		cl->GetGameState()->UpdatePackagePaths();
}
static void load_autorun_scripts(const std::function<void(const std::string &, std::vector<std::string> &)> &fFindFiles)
{
	std::vector<Game *> games;
	games.reserve(2);
	auto *sv = engine->GetServerNetworkState();
	if(sv != nullptr && sv->IsGameActive())
		games.push_back(sv->GetGameState());
	auto *cl = engine->GetClientState();
	if(cl != nullptr && cl->IsGameActive())
		games.push_back(cl->GetGameState());
	for(auto *game : games) {
		std::vector<std::string> files;
		fFindFiles(Lua::SCRIPT_DIRECTORY + "\\autorun\\*." + Lua::FILE_EXTENSION, files);

		for(auto &fName : files) {
			auto luaFileName = "autorun\\" + fName;
			game->ExecuteLuaFile(luaFileName);
		}

		files.clear();
		std::string gameDir = game->IsClient() ? "client" : "server";
		fFindFiles(Lua::SCRIPT_DIRECTORY + "\\autorun\\" + gameDir + "\\*." + Lua::FILE_EXTENSION, files);
		for(auto &fName : files) {
			auto luaFileName = "autorun\\" + gameDir + '\\' + fName;
			game->ExecuteLuaFile(luaFileName);
		}
	}
}

static bool is_addon_mounted(const std::string &addonPath, const std::vector<AddonInfo> &addons)
{
	auto path = "addons\\" + addonPath;
	auto it = std::find_if(addons.begin(), addons.end(), [&path](const AddonInfo &addonInfo) { return FileManager::ComparePath(addonInfo.GetLocalPath(), path); });
	return it != addons.end();
}

#ifdef _WIN32
static bool mount_linked_addon(const std::string &pathLink, std::vector<AddonInfo> &outAddons, bool silent = true)
{
	if(is_addon_mounted(pathLink, outAddons))
		return true;
	std::string resolvedPath;
	auto lnkPath = util::get_program_path() + "\\addons\\" + pathLink;
	ufile::remove_extension_from_filename(lnkPath);
	if(util::resolve_link(lnkPath, resolvedPath) == false) {
		Con::cwar << "Unable to resolve link path for '" << lnkPath << "'! This addon will not be mounted." << Con::endl;
		return false;
	}
	FileManager::AddCustomMountDirectory(resolvedPath.c_str(), true, static_cast<fsys::SearchFlags>(FSYS_SEARCH_ADDON));
	outAddons.push_back(AddonInfo("addons\\" + pathLink));
	load_autorun_scripts([&resolvedPath](const std::string &findTarget, std::vector<std::string> &outFiles) { FileManager::FindSystemFiles((resolvedPath + '\\' + findTarget).c_str(), &outFiles, nullptr); });
	if(silent == false)
		Con::cout << "Mounting linked addon '" << pathLink << "'..." << Con::endl;
	return true;
}
#endif

DirectoryWatcherCallback *AddonSystem::GetAddonWatcher() { return m_addonWatcher.get(); }

bool AddonSystem::MountAddon(const std::string &paddonPath, std::vector<AddonInfo> &outAddons, bool silent)
{
	// Valid addon paths are: addons/addonName, addons/addonName/addons/subAddonName, etc.
	auto addonPath = paddonPath;
	ustring::replace(addonPath, "/", "\\"); // TODO: We should be using forward slashes, not backward slashes for the normalized paths!
	auto path = util::Path::CreatePath(addonPath);
	auto n = path.GetComponentCount();
	if((n % 2) == 0)
		return false;
	if(n > 1) {
		size_t curOffset = 0;
		path.GetComponent(curOffset, &curOffset);
		for(auto i = decltype(n) {1u}; i < n; i += 2) {
			auto component = path.GetComponent(curOffset, &curOffset);
			if(component != "addons")
				return false; // This is not a top-level directory (i.e. it's not an addon, but the sub-directory of an addon)
			path.GetComponent(curOffset, &curOffset);
		}
	}

	if(is_addon_mounted(addonPath, outAddons))
		return true;
	auto fullPath = "addons\\" + addonPath;
	FileManager::AddCustomMountDirectory(fullPath.c_str(), static_cast<fsys::SearchFlags>(FSYS_SEARCH_ADDON));
	outAddons.push_back(AddonInfo(fullPath));
	update_package_paths();
	load_autorun_scripts([&fullPath](const std::string &findTarget, std::vector<std::string> &outFiles) { FileManager::FindFiles((fullPath + '\\' + findTarget).c_str(), &outFiles, nullptr); });
	if(silent == false)
		Con::cout << "Mounting addon '" << addonPath << "'..." << Con::endl;
	spdlog::info("Mounting addon '{}'...", addonPath);

	// The function calls above may have added other addons, so we need to search for our addon again
	auto it = std::find_if(outAddons.begin(), outAddons.end(), [&fullPath](const AddonInfo &addonInfo) { return addonInfo.GetLocalPath() == fullPath; });
	if(it != outAddons.end()) {
		// Inform the game states about the newly mounted addons
		auto &addonInfo = *it;
		auto *sv = engine->GetServerNetworkState();
		auto *cl = engine->GetClientState();
		std::vector<Game *> gameStates = {sv ? sv->GetGameState() : nullptr, cl ? cl->GetGameState() : nullptr};
		for(auto *game : gameStates) {
			if(!game)
				continue;
			game->InitializeMountedAddon(addonInfo);
		}
	}
	return true;
}
bool AddonSystem::MountAddon(const std::string &addonPath) { return MountAddon(addonPath, m_addons); }

void AddonSystem::MountAddons()
{
	std::vector<std::string> resFiles;
	std::vector<std::string> resDirs;
	FileManager::FindFiles("addons\\*", NULL, &resDirs);
	FileManager::FindFiles("addons\\*.pad", &resFiles, NULL);
	m_addons.reserve(resFiles.size() + resDirs.size());

	// Make sure that the "converted" addon is always the first in the mount order!
	// This addon contains asset files that were converted by the engine into a different
	// format (e.g. traditional texture assets to pbr assets).
	// The addon has to be the first, to allow it to override assets from other addons.
	// TODO: Do this properly and allow changing the mount order of all addons arbitrarily!
	auto itConverted = std::find(resDirs.begin(), resDirs.end(), "converted");
	if(itConverted != resDirs.end()) {
		resDirs.erase(itConverted);
		resDirs.insert(resDirs.begin(), "converted");
	}

	for(auto &d : resDirs)
		MountAddon(d, m_addons);

#ifdef _WIN32
	std::vector<std::string> resLinks;
	FileManager::FindFiles("addons\\*.lnk", &resLinks, NULL);
	for(auto &pathLink : resLinks)
		mount_linked_addon(pathLink, m_addons);
#endif

	for(auto &f : resFiles)
		LoadPADPackage("addons\\" + f);

	// Initialize watcher for new addons
	try {
		m_addonWatcher = std::make_shared<DirectoryWatcherCallback>(
		  "addons",
		  [](const std::string &fName) {
			  std::string ext;
			  if(ufile::get_extension(fName, &ext) == true) {
				  if(ustring::compare<std::string>(ext, "pad", false) == true) {
					  auto *pad = LoadPADPackage("addons\\" + fName);
					  if(pad != nullptr) {
						  auto *archFile = pad->GetArchiveFile();
						  if(archFile != nullptr) {
							  load_autorun_scripts([archFile](const std::string &findTarget, std::vector<std::string> &outFiles) {
								  std::vector<pragma::uva::FileInfo *> results;
								  archFile->SearchFiles(findTarget, results);
								  // archFile->SearchFiles(Lua::SCRIPT_DIRECTORY + "\\autorun\\*." + Lua::FILE_EXTENSION_PRECOMPILED, results);
								  outFiles.reserve(outFiles.size() + results.size());
								  for(auto *fi : results) {
									  if(fi->IsFile() == false)
										  continue;
									  outFiles.push_back(fi->name);
								  }
							  });
						  }
					  }
				  }
#ifdef _WIN32
				  else if(ustring::compare<std::string>(ext, "lnk", false) == true)
					  mount_linked_addon(fName, m_addons, false);
#endif
			  }
			  else {
				  // Directory
				  MountAddon(fName, m_addons, false);
			  }
		  },
		  DirectoryWatcherCallback::WatchFlags::WatchSubDirectories | DirectoryWatcherCallback::WatchFlags::WatchDirectoryChanges);
	}
	catch(const DirectoryWatcher::ConstructException &e) {
		Con::cwar << "[AddonSystem] Unable to watch addons directory: " << e.what() << Con::endl;
	}
}

void AddonSystem::Poll()
{
	if(m_addonWatcher == nullptr)
		return;
	m_addonWatcher->Poll();
}

void AddonSystem::UnmountAddons()
{
	m_addonWatcher = nullptr;
	FileManager::ClearCustomMountDirectories();
	FileManager::ClearPackages(static_cast<fsys::SearchFlags>(FSYS_SEARCH_ADDON));
	m_addons.clear();
}

const std::vector<AddonInfo> &AddonSystem::GetMountedAddons() { return m_addons; }

/////////////////////////////

AddonInfo::AddonInfo(const std::string &path, const util::Version &version, const std::string &uniqueId) : m_path(path), m_version(version), m_uniqueId(uniqueId) {}
const std::string &AddonInfo::GetLocalPath() const { return m_path; }
std::string AddonInfo::GetAbsolutePath() const
{
	std::string ext;
	if(ufile::get_extension(m_path, &ext) == false)
		return util::get_program_path() + '\\' + m_path;
#ifdef _WIN32
	std::string resolvedPath;
	auto lnkPath = util::get_program_path() + '\\' + m_path;
	ufile::remove_extension_from_filename(lnkPath);
	auto r = util::resolve_link(lnkPath, resolvedPath);
	if(r == false)
		return util::get_program_path() + '\\' + m_path;
	return resolvedPath;
#else
	auto path = util::get_program_path() + '/' + m_path;
	ufile::remove_extension_from_filename(path);
	return path;
#endif
}
const std::string &AddonInfo::GetUniqueId() const { return m_uniqueId; }
const util::Version &AddonInfo::GetVersion() const { return m_version; }
