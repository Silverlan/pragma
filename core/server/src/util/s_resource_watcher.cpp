/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_resource_watcher.hpp"
#include <pragma/ai/navsystem.h>
#include <sharedutils/util_file.h>

extern DLLSERVER SGame *s_game;

decltype(ESResourceWatcherCallbackType::NavMesh) ESResourceWatcherCallbackType::NavMesh = ESResourceWatcherCallbackType{umath::to_integral(E::NavMesh)};
decltype(ESResourceWatcherCallbackType::Count) ESResourceWatcherCallbackType::Count = ESResourceWatcherCallbackType{umath::to_integral(E::Count)};
void SResourceWatcherManager::OnResourceChanged(const util::Path &rootPath,const util::Path &path,const std::string &ext)
{
	ResourceWatcherManager::OnResourceChanged(rootPath,path,ext);
	auto &strPath = path.GetString();
	if((ext == pragma::nav::PNAV_EXTENSION_BINARY || ext == pragma::nav::PNAV_EXTENSION_ASCII) && s_game != nullptr)
	{
		auto fname = ufile::get_file_from_filename(strPath);
		ufile::remove_extension_from_filename(fname);
		auto mapName = s_game->GetMapName();
		if(ustring::compare(mapName,fname,false) == true)
		{
#if RESOURCE_WATCHER_VERBOSE > 0
			auto navPath = "maps\\" +strPath;
			Con::cout<<"[ResourceWatcher] Navigation mesh has changed: "<<navPath<<". Attempting to reload..."<<Con::endl;
#endif
			if(s_game->LoadNavMesh(true) == false)
			{
#if RESOURCE_WATCHER_VERBOSE > 0
			Con::cwar<<"WARNING: [ResourceWatcher] Unable to reload navigation mesh!"<<Con::endl;
#endif
			}
		}
		CallChangeCallbacks(ESResourceWatcherCallbackType::NavMesh,strPath,ext);
	}
}

