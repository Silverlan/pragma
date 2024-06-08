/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_DOC_HPP__
#define __LUA_DOC_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <vector>

#ifdef _WIN32
#define ENABLE_LAD 1
#endif

namespace pragma {
	namespace doc {
		class BaseCollectionObject;
	};
};
namespace Lua {
	namespace doc {
		static auto constexpr FILE_LOCATION = "doc/lua/";
		static auto constexpr FILE_EXTENSION_ASCII = "ldoc";
		static auto constexpr FILE_EXTENSION_BINARY = "ldoc_b";
		DLLNETWORK bool load_documentation_file(const std::string &fileName);
		DLLNETWORK void clear();
		DLLNETWORK void reset();
		DLLNETWORK void print_documentation(const std::string &name);
		DLLNETWORK void generate_autocomplete_script();
		DLLNETWORK void find_candidates(const std::string &name, std::vector<const pragma::doc::BaseCollectionObject *> &outCandidates, uint32_t candidateLimit);
	};
};

#endif
