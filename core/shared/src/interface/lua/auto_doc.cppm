// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.auto_doc;

export import pragma.doc;
export import std.compat;

export namespace Lua {
	namespace doc {
		auto constexpr FILE_LOCATION = "doc/lua/";
		auto constexpr FILE_EXTENSION_ASCII = "ldoc";
		auto constexpr FILE_EXTENSION_BINARY = "ldoc_b";
		DLLNETWORK bool load_documentation_file(const std::string &fileName);
		DLLNETWORK void clear();
		DLLNETWORK void reset();
		DLLNETWORK void print_documentation(const std::string &name, std::stringstream &ss);
		DLLNETWORK void generate_autocomplete_script();
		DLLNETWORK void find_candidates(const std::string &name, std::vector<const pragma::doc::BaseCollectionObject *> &outCandidates, uint32_t candidateLimit);
	};
};
