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
	};
};

#endif
