// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.string;

export import pragma.lua;

export namespace Lua {
	namespace string {
		DLLNETWORK std::string snake_case_to_camel_case(const std::string &str);
		DLLNETWORK std::string camel_case_to_snake_case(const std::string &str);
		DLLNETWORK uint32_t calc_levenshtein_distance(const std::string &s0, const std::string &s1);
		DLLNETWORK double calc_levenshtein_similarity(const std::string &s0, const std::string &s1);
		DLLNETWORK void find_longest_common_substring(const std::string &s0, const std::string &s1, size_t &outStartIdx, size_t &outLen, size_t &outEndIdx);
		DLLNETWORK luabind::object split(lua::State *l, const std::string &str, const std::string &delimiter);
		DLLNETWORK std::string join(lua::State *l, luabind::table<> values, const std::string &joinChar = ";");
		DLLNETWORK std::string remove_whitespace(const std::string &s);
		DLLNETWORK std::string remove_quotes(const std::string &s);
		DLLNETWORK void find_similar_elements(lua::State *l, const std::string &baseElement, luabind::table<> elements, uint32_t limit, luabind::object &outSimilarElements, luabind::object &outSimilarities);
	};
};
