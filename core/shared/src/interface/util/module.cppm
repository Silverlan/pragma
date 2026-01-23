// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.umodule;

export import pragma.util;

export namespace pragma::util {
	DLLNETWORK std::string get_normalized_module_path(const std::string &lib, std::optional<bool> checkForClientSide = {});
	DLLNETWORK std::shared_ptr<Library> load_library_module(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::optional<bool> checkForClientSide = {}, std::string *err = nullptr);
	DLLNETWORK std::vector<std::string> get_default_additional_library_search_directories(const std::string &libModulePath);
};
