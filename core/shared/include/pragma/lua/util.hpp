/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_UTIL_HPP__
#define __LUA_UTIL_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua {
	DLLNETWORK luabind::detail::class_rep *get_crep(luabind::object o);
	DLLNETWORK void set_ignore_include_cache(bool b);
	DLLNETWORK std::optional<std::string> find_script_file(const std::string &fileName);

	template<class T, class TCast>
	bool get_table_value(lua_State *l, const std::string &name, uint32_t t, TCast &ret, const std::function<T(lua_State *, int32_t)> &check)
	{
		Lua::PushString(l, name); /* 1 */
		Lua::GetTableValue(l, t);
		auto r = false;
		if(Lua::IsSet(l, -1) == true) {
			ret = static_cast<TCast>(check(l, -1));
			r = true;
		}
		Lua::Pop(l, 1); /* 0 */
		return r;
	}
	template<typename T>
	void get_table_values(lua_State *l, uint32_t tIdx, std::vector<T> &values, const std::function<T(lua_State *, int32_t)> &tCheck)
	{
		Lua::CheckTable(l, tIdx);
		auto numOffsets = Lua::GetObjectLength(l, tIdx);
		values.reserve(numOffsets);
		for(auto i = decltype(numOffsets) {0}; i < numOffsets; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, tIdx);

			values.push_back(tCheck(l, -1));
			Lua::Pop(l, 1);
		}
	}
	template<typename T>
	std::vector<T> get_table_values(lua_State *l, uint32_t tIdx, const std::function<T(lua_State *, int32_t)> &tCheck)
	{
		std::vector<T> values;
		get_table_values(l, tIdx, values, tCheck);
		return values;
	}
};

#endif
