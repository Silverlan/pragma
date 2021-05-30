/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/networking/networking_modules.hpp"
#include <fsys/filesystem.h>

std::vector<std::string> pragma::networking::GetAvailableNetworkingModules()
{
	std::vector<std::string> dirs {};
	FileManager::FindFiles(
		"modules/networking/*",nullptr,&dirs,
		fsys::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::networking::GetNetworkingModuleLocation(const std::string &netModule,bool server)
{
	return "networking/" +netModule +"/pr_" +netModule +(server ? "_server" : "_client");
}
