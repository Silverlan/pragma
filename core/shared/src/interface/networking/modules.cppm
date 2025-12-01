// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.modules;

export import std;

export namespace pragma::networking {
	constexpr std::string_view NET_LIBRARY_MODULE_LOCATION = "networking";
	DLLNETWORK std::vector<std::string> GetAvailableNetworkingModules();
	DLLNETWORK std::string GetNetworkingModuleLocation(const std::string &netModule, bool server);
};
