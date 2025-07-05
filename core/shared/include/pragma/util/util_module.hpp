// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_MODULE_HPP__
#define __UTIL_MODULE_HPP__

#include "pragma/definitions.h"
#include <string>
#include <optional>
#include <vector>

namespace util {
	class Library;
	DLLNETWORK std::string get_normalized_module_path(const std::string &lib, std::optional<bool> checkForClientSide = {});
	DLLNETWORK std::shared_ptr<util::Library> load_library_module(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::optional<bool> checkForClientSide = {}, std::string *err = nullptr);
	DLLNETWORK std::vector<std::string> get_default_additional_library_search_directories(const std::string &libModulePath);
};

#endif
