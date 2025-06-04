/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

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
