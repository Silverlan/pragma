// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#define ENABLE_LAD 1
#endif

export module pragma.client:scripting.lua.auto_doc;

export namespace Lua {
	namespace doc {
		static auto constexpr FILE_LOCATION = "doc/lua/";
		static auto constexpr FILE_EXTENSION_ASCII = "ldoc";
		static auto constexpr FILE_EXTENSION_BINARY = "ldoc_b";
		DLLNETWORK bool load_documentation_file(const std::string &fileName);
		DLLNETWORK void clear();
		DLLNETWORK void reset();
		DLLNETWORK void print_documentation(const std::string &name, std::stringstream &ss);
		DLLNETWORK void generate_autocomplete_script();
		DLLNETWORK void find_candidates(const std::string &name, std::vector<const pragma::doc::BaseCollectionObject *> &outCandidates, uint32_t candidateLimit);
	};
};
