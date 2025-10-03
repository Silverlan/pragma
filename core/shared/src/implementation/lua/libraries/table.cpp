// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

module pragma.shared;

import :scripting.lua.libraries.table;

int Lua::table::has_value(lua_State *l)
{
	Lua::CheckTable(l, 1);

	Lua::PushValue(l, 1); /* 1 */
	auto t = Lua::GetStackTop(l);

	Lua::PushNil(l);                   /* 2 */
	while(Lua::GetNextPair(l, t) != 0) /* 3 */
	{
		// TODO: Compare values
		Lua::Pop(l, 1); /* 2 */
	} /* 1 */

	Lua::Pop(l, 1); /* 0 */

	Lua::PushBool(l, false);
	return 1;
}

int Lua::table::random(lua_State *l)
{
	Lua::CheckTable(l, 1);
	auto n = Lua::GetObjectLength(l, 1);
	if(n == 0)
		return 0;
	auto r = umath::random(1, static_cast<int32_t>(n));
	Lua::PushInt(l, r);
	Lua::GetTableValue(l, 1);
	return 1;
}

int Lua::table::is_empty(lua_State *l)
{
	Lua::CheckTable(l, 1);
	luabind::object t {l, 1};

	luabind::iterator it {t};
	auto nonEmpty = (it != luabind::iterator {});
	Lua::PushBool(l, !nonEmpty);
	return 1;
}
int Lua::table::count(lua_State *l)
{
	Lua::CheckTable(l, 1);
	luabind::object t {l, 1};
	uint32_t count = 0;
	for(luabind::iterator it {t}, end; it != end; ++it)
		++count;
	Lua::PushInt(l, count);
	return 1;
}
