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
		DLLNETWORK int32_t calc_levenshtein_distance(lua_State *l);
		DLLNETWORK int32_t calc_levenshtein_similarity(lua_State *l);
		DLLNETWORK int32_t find_longest_common_substring(lua_State *l);
		DLLNETWORK int32_t split(lua_State *l);
		DLLNETWORK int32_t join(lua_State *l);
		DLLNETWORK int32_t remove_whitespace(lua_State *l);
		DLLNETWORK int32_t remove_quotes(lua_State *l);
		DLLNETWORK int32_t find_similar_elements(lua_State *l);
		DLLNETWORK int32_t is_integer(lua_State *l);
		DLLNETWORK int32_t is_number(lua_State *l);
	};
};

#endif
