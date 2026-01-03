// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :core.resource_watcher;

import :game;

decltype(pragma::util::ESResourceWatcherCallbackType::NavMesh) pragma::util::ESResourceWatcherCallbackType::NavMesh = ESResourceWatcherCallbackType {math::to_integral(E::NavMesh)};
decltype(pragma::util::ESResourceWatcherCallbackType::Count) pragma::util::ESResourceWatcherCallbackType::Count = ESResourceWatcherCallbackType {math::to_integral(E::Count)};
void pragma::util::SResourceWatcherManager::OnResourceChanged(const Path &rootPath, const Path &path, const std::string &ext)
{
	ResourceWatcherManager::OnResourceChanged(rootPath, path, ext);
	auto &strPath = path.GetString();
	if((ext == nav::PNAV_EXTENSION_BINARY || ext == nav::PNAV_EXTENSION_ASCII) && SGame::Get() != nullptr) {
		auto fname = ufile::get_file_from_filename(strPath);
		ufile::remove_extension_from_filename(fname);
		auto mapName = SGame::Get()->GetMapName();
		if(string::compare(mapName, fname, false) == true) {
#if RESOURCE_WATCHER_VERBOSE > 0
			auto navPath = "maps\\" + strPath;
			Con::COUT << "[ResourceWatcher] Navigation mesh has changed: " << navPath << ". Attempting to reload..." << Con::endl;
#endif
			if(SGame::Get()->LoadNavMesh(true) == false) {
#if RESOURCE_WATCHER_VERBOSE > 0
				Con::CWAR << "[ResourceWatcher] Unable to reload navigation mesh!" << Con::endl;
#endif
			}
		}
		CallChangeCallbacks(ESResourceWatcherCallbackType::NavMesh, strPath, ext);
	}
}
