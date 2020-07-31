/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/lua/luaapi.h"
#include "pragma/lua/lua_error_handling.hpp"
#include "pragma/console/conout.h"
#include <luainterface.hpp>
#include <luabind/class_info.hpp>
#include <luabind/function_introspection.hpp>

static auto s_bExtendedModules = false;
void Lua::set_extended_lua_modules_enabled(bool b) {s_bExtendedModules = b;}
bool Lua::get_extended_lua_modules_enabled() {return s_bExtendedModules;}

void Lua::initialize_lua_state(Lua::Interface &lua)
{
	// See http://www.lua.org/source/5.3/linit.c.html
	auto *l = lua.GetState();
	std::vector<luaL_Reg> loadedLibs = {
		{"_G",luaopen_base},

#ifndef USE_LUAJIT
		{LUA_UTF8LIBNAME,luaopen_utf8},
		{LUA_COLIBNAME,luaopen_coroutine},
#else
		// coroutine already included in base!
		{LUA_BITLIBNAME,luaopen_bit},
#endif
		{LUA_TABLIBNAME,luaopen_table},
		{LUA_OSLIBNAME,luaopen_os},
		{LUA_STRLIBNAME,luaopen_string},
		{LUA_MATHLIBNAME,luaopen_math},
		{LUA_DBLIBNAME,luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
		{LUA_BITLIBNAME,luaopen_bit32},
#endif
	};
	if(s_bExtendedModules == true)
	{
		loadedLibs.push_back({LUA_LOADLIBNAME,luaopen_package});
		loadedLibs.push_back({LUA_IOLIBNAME,luaopen_io});
	}
	for(auto &lib : loadedLibs)
	{
#ifdef USE_LUAJIT
		lua_pushcfunction(l,lib.func);
		lua_pushlstring(l,lib.name,strlen(lib.name));
		lua_call(l,1,0);
#else
		luaL_requiref(l,lib->name,lib->func,1);
		lua_pop(l,1);  // remove lib
#endif
	}
	luabind::open(l);
	if(s_bExtendedModules)
	{
		luabind::bind_class_info(l);
		luabind::bind_function_introspection(l);
	}
	Lua::initialize_error_handler();
	lua_atpanic(l,[](lua_State *l) -> int32_t {
		Lua::HandleLuaError(l);
		Con::crit<<"ERROR: Lua Panic!"<<Con::endl;
		return 0;
	});
}
