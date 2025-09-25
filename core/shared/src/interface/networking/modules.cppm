// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string_view>
#include <vector>

export module pragma.shared:networking.modules;

export namespace pragma::networking {
	static constexpr std::string_view NET_LIBRARY_MODULE_LOCATION = "networking";
	DLLNETWORK std::vector<std::string> GetAvailableNetworkingModules();
	DLLNETWORK std::string GetNetworkingModuleLocation(const std::string &netModule, bool server);
};
