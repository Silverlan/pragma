// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <vector>
#include <sharedutils/util_library.hpp>

export module pragma.shared:util.module;

export namespace util {
	DLLNETWORK std::string get_normalized_module_path(const std::string &lib, std::optional<bool> checkForClientSide = {});
	DLLNETWORK std::shared_ptr<util::Library> load_library_module(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::optional<bool> checkForClientSide = {}, std::string *err = nullptr);
	DLLNETWORK std::vector<std::string> get_default_additional_library_search_directories(const std::string &libModulePath);
};
