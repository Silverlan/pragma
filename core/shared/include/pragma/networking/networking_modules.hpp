/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __NETWORKING_MODULES_HPP__
#define __NETWORKING_MODULES_HPP__

#include "pragma/networkdefinitions.h"
#include <string_view>
#include <vector>

namespace pragma::networking
{
	static constexpr std::string_view NET_LIBRARY_MODULE_LOCATION = "networking";
	DLLNETWORK std::vector<std::string> GetAvailableNetworkingModules();
	DLLNETWORK std::string GetNetworkingModuleLocation(const std::string &netModule,bool server);
};

#endif
