// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_STEAM_HPP__
#define __UTIL_STEAM_HPP__

#include "pragma/networkdefinitions.h"
#include "sharedutils/util_path.hpp"

namespace util::steam {
	DLLNETWORK std::optional<std::string> find_steam_installation_path();
	DLLNETWORK std::vector<util::Path> find_steam_root_paths();
	DLLNETWORK bool get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations);
}

#endif
