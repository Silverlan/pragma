// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.steam;

export import :util.steam.vdf;

export namespace pragma::util::steam {
	DLLNETWORK std::optional<std::string> find_steam_installation_path();
	DLLNETWORK std::vector<Path> find_steam_root_paths();
	DLLNETWORK bool get_external_steam_locations(const std::string &steamRootPath, std::vector<std::string> &outExtLocations);
}
