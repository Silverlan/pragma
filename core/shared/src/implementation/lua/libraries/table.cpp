// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.table;

int Lua::table::has_value(lua::State *l)
{
	CheckTable(l, 1);

	PushValue(l, 1); /* 1 */
	auto t = GetStackTop(l);

	PushNil(l);                   /* 2 */
	while(GetNextPair(l, t) != 0) /* 3 */
	{
		// TODO: Compare values
		Pop(l, 1); /* 2 */
	} /* 1 */

	Pop(l, 1); /* 0 */

	PushBool(l, false);
	return 1;
}

int Lua::table::random(lua::State *l)
{
	CheckTable(l, 1);
	auto n = GetObjectLength(l, 1);
	if(n == 0)
		return 0;
	auto r = pragma::math::random(1, static_cast<int32_t>(n));
	PushInt(l, r);
	GetTableValue(l, 1);
	return 1;
}

int Lua::table::is_empty(lua::State *l)
{
	CheckTable(l, 1);
	luabind::object t {l, 1};

	luabind::iterator it {t};
	auto nonEmpty = (it != luabind::iterator {});
	PushBool(l, !nonEmpty);
	return 1;
}
int Lua::table::count(lua::State *l)
{
	CheckTable(l, 1);
	luabind::object t {l, 1};
	uint32_t count = 0;
	for(luabind::iterator it {t}, end; it != end; ++it)
		++count;
	PushInt(l, count);
	return 1;
}
