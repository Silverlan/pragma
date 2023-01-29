/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2021 Silverlan
*/

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
