// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.modules;

std::vector<std::string> pragma::networking::GetAvailableNetworkingModules()
{
	std::vector<std::string> dirs {};
	fs::find_files("modules/networking/*", nullptr, &dirs,
	  fs::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::networking::GetNetworkingModuleLocation(const std::string &netModule, bool server) { return "networking/" + netModule + "/pr_" + netModule + (server ? "_server" : "_client"); }
