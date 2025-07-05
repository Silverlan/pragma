// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __NETWORKING_MODULES_HPP__
#define __NETWORKING_MODULES_HPP__

#include "pragma/networkdefinitions.h"
#include <string_view>
#include <vector>

namespace pragma::networking {
	static constexpr std::string_view NET_LIBRARY_MODULE_LOCATION = "networking";
	DLLNETWORK std::vector<std::string> GetAvailableNetworkingModules();
	DLLNETWORK std::string GetNetworkingModuleLocation(const std::string &netModule, bool server);
};

#endif
