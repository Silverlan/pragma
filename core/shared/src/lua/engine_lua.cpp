/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/luaapi.h"
#include "pragma/lua/lua_error_handling.hpp"
#include "pragma/console/conout.h"
#include <luainterface.hpp>
#include <luabind/class_info.hpp>
#include <luabind/function_introspection.hpp>

static auto s_bExtendedModules = false;
void Lua::set_extended_lua_modules_enabled(bool b) { s_bExtendedModules = b; }
bool Lua::get_extended_lua_modules_enabled() { return s_bExtendedModules; }

DLLNETWORK const Lua::object Lua::nil {};

void Lua::initialize_lua_state(Lua::Interface &lua)
{
	// See http://www.lua.org/source/5.3/linit.c.html
	auto *l = lua.GetState();
	std::vector<luaL_Reg> loadedLibs
	  = { {"_G", luaopen_base},

#ifndef USE_LUAJIT
		    {LUA_UTF8LIBNAME, luaopen_utf8},
		    {LUA_COLIBNAME, luaopen_coroutine},
#else
		    // coroutine already included in base!
		    {LUA_BITLIBNAME, luaopen_bit},
		    {LUA_JITLIBNAME, luaopen_jit},
#endif
		    {LUA_TABLIBNAME, luaopen_table},
		    {LUA_OSLIBNAME, luaopen_os},
		    {LUA_STRLIBNAME, luaopen_string},
		    {LUA_MATHLIBNAME, luaopen_math},
		    {LUA_DBLIBNAME, luaopen_debug},
#if defined(LUA_COMPAT_BITLIB)
		    {LUA_BITLIBNAME, luaopen_bit32},
#endif
	    };
	loadedLibs.push_back({LUA_LOADLIBNAME, luaopen_package});
	if(s_bExtendedModules == true) {
		loadedLibs.push_back({LUA_IOLIBNAME, luaopen_io});
	}
	for(auto &lib : loadedLibs) {
#ifdef USE_LUAJIT
		lua_pushcfunction(l, lib.func);
		lua_pushlstring(l, lib.name, strlen(lib.name));
		lua_call(l, 1, 0);
#else
		luaL_requiref(l, lib->name, lib->func, 1);
		lua_pop(l, 1); // remove lib
#endif
	}
	luabind::open(l);
	if(s_bExtendedModules) {
		luabind::bind_class_info(l);
		luabind::bind_function_introspection(l);
	}
	Lua::initialize_error_handler();
	lua_atpanic(l, [](lua_State *l) -> int32_t {
		Lua::HandleLuaError(l);
		Con::crit << "Lua Panic!" << Con::endl;
		return 0;
	});
}

static void dump_traceback(bool cl)
{
	auto *en = pragma::get_engine();
	auto *state = en ? (cl ? en->GetClientState() : en->GetServerNetworkState()) : nullptr;
	auto *g = cl ? state->GetGameState() : nullptr;
	if(!g)
		return;
	Lua::PrintTraceback(g->GetLuaState());
}
static void dump_stack(bool cl)
{
	auto *en = pragma::get_engine();
	auto *state = en ? (cl ? en->GetClientState() : en->GetServerNetworkState()) : nullptr;
	auto *g = cl ? state->GetGameState() : nullptr;
	if(!g)
		return;
	Lua::StackDump(g->GetLuaState());
}
namespace pragma::lua::debug {
	// These are mainly used in the VS immediate window for debugging purposes
	DLLNETWORK void dump_traceback_cl() { ::dump_traceback(true); }
	DLLNETWORK void dump_traceback_sv() { ::dump_traceback(false); }
	DLLNETWORK void dump_stack_cl() { ::dump_stack(true); }
	DLLNETWORK void dump_stack_sv() { ::dump_stack(false); }
};
