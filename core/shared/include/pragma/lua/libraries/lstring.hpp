/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LSTRING_HPP__
#define __LSTRING_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace string
	{
		DLLNETWORK uint32_t calc_levenshtein_distance(const std::string &s0,const std::string &s1);
		DLLNETWORK double calc_levenshtein_similarity(const std::string &s0,const std::string &s1);
		DLLNETWORK void find_longest_common_substring(const std::string &s0,const std::string &s1,size_t &outStartIdx,size_t &outLen,size_t &outEndIdx);
		DLLNETWORK std::vector<std::string> split(lua_State *l,const std::string &str,const std::string &delimiter);
		DLLNETWORK std::string join(lua_State *l,luabind::table<> values,const std::string &joinChar=";");
		DLLNETWORK std::string remove_whitespace(const std::string &s);
		DLLNETWORK std::string remove_quotes(const std::string &s);
		DLLNETWORK void find_similar_elements(lua_State *l,const std::string &baseElement,luabind::table<> elements,uint32_t limit,luabind::object &outSimilarElements,luabind::object &outSimilarities);
	};
};

#endif
