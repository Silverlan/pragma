// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef _WIN32
#endif

module pragma.shared;

import :core.addon_system;

import pragma.uva;
import pragma.pad;

decltype(pragma::AddonSystem::m_addons) pragma::AddonSystem::m_addons;
decltype(pragma::AddonSystem::m_addonWatcher) pragma::AddonSystem::m_addonWatcher = nullptr;

pragma::pad::PADPackage *pragma::AddonSystem::LoadPADPackage(const std::string &path)
{
	auto it = std::find_if(m_addons.begin(), m_addons.end(), [&path](const AddonInfo &addon) { return fs::compare_path(addon.GetLocalPath(), path); });
	if(it != m_addons.end())
		return nullptr;
	auto *package = dynamic_cast<pad::PADPackage *>(pragma::fs::load_package(path, static_cast<fs::SearchFlags>(FSYS_SEARCH_ADDON)));
	if(package != nullptr)
		m_addons.push_back(AddonInfo(path, package->GetPackageVersion(), package->GetPackageId()));
	return package;
}

static void update_package_paths()
{

	auto *sv = pragma::Engine::Get()->GetServerNetworkState();
	if(sv != nullptr && sv->IsGameActive())
		sv->GetGameState()->UpdatePackagePaths();
	auto *cl = pragma::Engine::Get()->GetClientState();
	if(cl != nullptr && cl->IsGameActive())
		cl->GetGameState()->UpdatePackagePaths();
}
static void load_autorun_scripts(const std::function<void(const std::string &, std::vector<std::string> &)> &fFindFiles)
{
	std::vector<pragma::Game *> games;
	games.reserve(2);
	auto *sv = pragma::Engine::Get()->GetServerNetworkState();
	if(sv != nullptr && sv->IsGameActive())
		games.push_back(sv->GetGameState());
	auto *cl = pragma::Engine::Get()->GetClientState();
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

static bool is_addon_mounted(const std::string &addonPath, const std::vector<pragma::AddonInfo> &addons)
{
	auto path = "addons\\" + addonPath;
	auto it = std::find_if(addons.begin(), addons.end(), [&path](const pragma::AddonInfo &addonInfo) { return pragma::fs::compare_path(addonInfo.GetLocalPath(), path); });
	return it != addons.end();
}

#ifdef _WIN32
static bool mount_linked_addon(const std::string &pathLink, std::vector<pragma::AddonInfo> &outAddons, bool silent = true)
{
	if(is_addon_mounted(pathLink, outAddons))
		return true;
	std::string resolvedPath;
	std::string lnkPath;
	if(!pragma::fs::find_absolute_path(pragma::util::DirPath("addons", pathLink).GetString(), lnkPath))
		return false;
	ufile::remove_extension_from_filename(lnkPath);
	if(pragma::util::resolve_link(lnkPath, resolvedPath) == false) {
		Con::CWAR << "Unable to resolve link path for '" << lnkPath << "'! This addon will not be mounted." << Con::endl;
		return false;
	}
	pragma::fs::add_custom_mount_directory(resolvedPath, true, static_cast<pragma::fs::SearchFlags>(pragma::FSYS_SEARCH_ADDON));
	outAddons.push_back(pragma::AddonInfo("addons\\" + pathLink));
	load_autorun_scripts([&resolvedPath](const std::string &findTarget, std::vector<std::string> &outFiles) { pragma::fs::find_system_files((resolvedPath + '\\' + findTarget), &outFiles, nullptr); });
	if(silent == false)
		Con::COUT << "Mounting linked addon '" << pathLink << "'..." << Con::endl;
	return true;
}
#endif

pragma::fs::DirectoryWatcherCallback *pragma::AddonSystem::GetAddonWatcher() { return m_addonWatcher.get(); }

bool pragma::AddonSystem::MountAddon(const std::string &paddonPath, std::vector<AddonInfo> &outAddons, bool silent)
{
	// Valid addon paths are: addons/addonName, addons/addonName/addons/subAddonName, etc.
	auto addonPath = paddonPath;
	string::replace(addonPath, "/", "\\"); // TODO: We should be using forward slashes, not backward slashes for the normalized paths!
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
	fs::add_custom_mount_directory(fullPath, static_cast<fs::SearchFlags>(FSYS_SEARCH_ADDON));
	outAddons.push_back(AddonInfo(fullPath));
	update_package_paths();
	load_autorun_scripts([&fullPath](const std::string &findTarget, std::vector<std::string> &outFiles) { fs::find_files((fullPath + '\\' + findTarget), &outFiles, nullptr); });
	if(silent == false)
		Con::COUT << "Mounting addon '" << addonPath << "'..." << Con::endl;
	spdlog::info("Mounting addon '{}'...", addonPath);

	// The function calls above may have added other addons, so we need to search for our addon again
	auto it = std::find_if(outAddons.begin(), outAddons.end(), [&fullPath](const AddonInfo &addonInfo) { return addonInfo.GetLocalPath() == fullPath; });
	if(it != outAddons.end()) {
		// Inform the game states about the newly mounted addons
		auto &addonInfo = *it;
		auto *sv = Engine::Get()->GetServerNetworkState();
		auto *cl = Engine::Get()->GetClientState();
		std::vector<Game *> gameStates = {sv ? sv->GetGameState() : nullptr, cl ? cl->GetGameState() : nullptr};
		for(auto *game : gameStates) {
			if(!game)
				continue;
			game->InitializeMountedAddon(addonInfo);
		}
	}
	return true;
}
bool pragma::AddonSystem::MountAddon(const std::string &addonPath) { return MountAddon(addonPath, m_addons); }

void pragma::AddonSystem::MountAddons()
{
	std::vector<std::string> resFiles;
	std::vector<std::string> resDirs;
	fs::find_files("addons\\*", nullptr, &resDirs);
	fs::find_files("addons\\*.pad", &resFiles, nullptr);
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
	fs::find_files("addons\\*.lnk", &resLinks, nullptr);
	for(auto &pathLink : resLinks)
		mount_linked_addon(pathLink, m_addons);
#endif

	for(auto &f : resFiles)
		LoadPADPackage("addons\\" + f);

	// Initialize watcher for new addons
	try {
		m_addonWatcher = pragma::util::make_shared<fs::DirectoryWatcherCallback>(
		  "addons",
		  [](const std::string &fName) {
			  std::string ext;
			  if(ufile::get_extension(fName, &ext) == true) {
				  if(pragma::string::compare<std::string>(ext, "pad", false) == true) {
					  auto *pad = LoadPADPackage("addons\\" + fName);
					  if(pad != nullptr) {
						  auto *archFile = pad->GetArchiveFile();
						  if(archFile != nullptr) {
							  load_autorun_scripts([archFile](const std::string &findTarget, std::vector<std::string> &outFiles) {
								  std::vector<uva::FileInfo *> results;
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
				  else if(pragma::string::compare<std::string>(ext, "lnk", false) == true)
					  mount_linked_addon(fName, m_addons, false);
#endif
			  }
			  else {
				  // Directory
				  MountAddon(fName, m_addons, false);
			  }
		  },
		  fs::DirectoryWatcherCallback::WatchFlags::WatchDirectoryChanges);
	}
	catch(const fs::DirectoryWatcher::ConstructException &e) {
		Con::CWAR << "[AddonSystem] Unable to watch addons directory: " << e.what() << Con::endl;
	}
}

void pragma::AddonSystem::Poll()
{
	if(m_addonWatcher == nullptr)
		return;
	m_addonWatcher->Poll();
}

void pragma::AddonSystem::UnmountAddons()
{
	m_addonWatcher = nullptr;
	fs::clear_custom_mount_directories();
	fs::clear_packages(static_cast<fs::SearchFlags>(FSYS_SEARCH_ADDON));
	m_addons.clear();
}

const std::vector<pragma::AddonInfo> &pragma::AddonSystem::GetMountedAddons() { return m_addons; }

/////////////////////////////

pragma::AddonInfo::AddonInfo(const std::string &path, const util::Version &version, const std::string &uniqueId) : m_path(path), m_version(version), m_uniqueId(uniqueId) {}
const std::string &pragma::AddonInfo::GetLocalPath() const { return m_path; }
std::string pragma::AddonInfo::GetAbsolutePath() const
{
	std::string absPath;
	if(!fs::find_absolute_path(m_path, absPath))
		return util::DirPath(fs::get_program_path(), m_path).GetString();

	std::string ext;
	if(ufile::get_extension(absPath, &ext) == false)
		return absPath;
#ifdef _WIN32
	std::string resolvedPath;
	auto lnkPath = absPath;
	ufile::remove_extension_from_filename(lnkPath);
	auto r = pragma::util::resolve_link(lnkPath, resolvedPath);
	if(r == false)
		return absPath;
	return resolvedPath;
#else
	ufile::remove_extension_from_filename(absPath);
	return absPath;
#endif
}
const std::string &pragma::AddonInfo::GetUniqueId() const { return m_uniqueId; }
const pragma::util::Version &pragma::AddonInfo::GetVersion() const { return m_version; }
