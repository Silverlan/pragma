// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include "pragma/lua/ldefinitions.h"
#include "spdlog/spdlog.h"

module pragma.shared;

import :scripting.lua.api;

static auto s_bExtendedModules = false;
void Lua::set_extended_lua_modules_enabled(bool b) { s_bExtendedModules = b; }
bool Lua::get_extended_lua_modules_enabled() { return s_bExtendedModules; }

DLLNETWORK const Lua::object Lua::nil {};

void Lua::initialize_lua_state(Lua::Interface &lua)
{
	// See http://www.lua.org/source/5.3/linit.c.html
	auto *l = lua.GetState();
	std::vector<luaL_Reg> loadedLibs = {
	  {"_G", lua::open_base},

#ifndef USE_LUAJIT
	  {lua::LIB_UTF8, lua::open_utf8},
	  {lua::LIB_COROUTINE, lua::open_coroutine},
#else
	  // coroutine already included in base!
	  {lua::LIB_BIT, lua::open_bit},
	  {lua::LIB_JIT, lua::open_jit},
#endif
	  {lua::LIB_TABLE, lua::open_table},
	  {lua::LIB_OS, lua::open_os},
	  {lua::LIB_STRING, lua::open_string},
	  {lua::LIB_MATH, lua::open_math},
	  {lua::LIB_DEBUG, lua::open_debug},
#if defined(LUA_COMPAT_BITLIB)
	  {lua::LIB_BIT, lua::open_bit32},
#endif
	};
	loadedLibs.push_back({lua::LIB_PACKAGE, lua::open_package});
	if(s_bExtendedModules == true) {
		loadedLibs.push_back({lua::LIB_IO, lua::open_io});
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
		spdlog::get("lua")->critical("Lua Panic!");
		::pragma::debug::generate_crash_dump();
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
	// These are mainly used in the immediate window for debugging purposes
	DLLNETWORK void dump_traceback_cl() { ::dump_traceback(true); }
	DLLNETWORK void dump_traceback_sv() { ::dump_traceback(false); }
	DLLNETWORK void dump_stack_cl() { ::dump_stack(true); }
	DLLNETWORK void dump_stack_sv() { ::dump_stack(false); }
};
