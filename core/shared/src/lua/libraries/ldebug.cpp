// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/console/conout.h>
#include <interface/scripting/lua/lua.hpp>
#include <algorithm>
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
// required for beep
//#include <ncurses.h>
#endif

//import pragma.scripting.lua;

int Lua::debug::collectgarbage(lua_State *l)
{
	// Calling twice on purpose: https://stackoverflow.com/a/28320364/2482983
	std::string err;
	Lua::RunString(l, "collectgarbage()", "internal", err);
	Lua::RunString(l, "collectgarbage()", "internal", err);
	return 0;
}
void Lua::debug::stackdump(lua_State *l)
{
	int top = lua_gettop(l);
	Con::cout << "Total in stack: " << top << Con::endl;
	for(int i = 1; i <= top; i++) {
		int t = lua_type(l, i);
		switch(t) {
		case LUA_TSTRING:
			Con::cout << "string: " << lua_tostring(l, i) << Con::endl;
			break;
		case LUA_TBOOLEAN:
			Con::cout << "boolean: " << (lua_toboolean(l, i) ? "true" : "false") << Con::endl;
			break;
		case LUA_TNUMBER:
			Con::cout << "number: " << lua_tonumber(l, i) << Con::endl;
			break;
		default:
			Con::cout << lua_typename(l, i) << Con::endl;
			break;
		}
	}
	if(top > 0)
		Con::cout << Con::endl;
}

void Lua::debug::beep(lua_State *l)
{
	int ms = 500;
	int freq = 523;
#ifdef _WIN32
	Beep(freq, ms);
#else
	// TODO: This does only work clientside.
	std::vector<std::string> argv {"beep"};
	pragma::get_engine()->RunConsoleCommand("sound_play", argv);

	// Does not work
	//initscr();
	//::beep();
	//endwin();
#endif
}

std::string Lua::debug::move_state_to_string(lua_State *l, pragma::BaseAIComponent::MoveResult v)
{
	switch(v) {
	case pragma::BaseAIComponent::MoveResult::TargetUnreachable:
		return "ai.MOVE_STATE_TARGET_UNREACHABLE";
	case pragma::BaseAIComponent::MoveResult::TargetReached:
		return "ai.MOVE_STATE_TARGET_REACHED";
	case pragma::BaseAIComponent::MoveResult::WaitingForPath:
		return "ai.MOVE_STATE_WAITING_FOR_PATH";
	case pragma::BaseAIComponent::MoveResult::MovingToTarget:
		return "ai.MOVE_STATE_MOVING_TO_TARGET";
	default:
		return "";
	}
}

void Lua::debug::enable_remote_debugging(lua_State *l)
{
	if(get_extended_lua_modules_enabled() == false) {
		Con::cwar << "Unable to enable remote debugging: Game has to be started with -luaext launch parameter!" << Con::endl;
		return;
	}

	Lua::GetGlobal(l, "require");
	Lua::PushString(l, "modules/mobdebug"); // Note: This will disable jit!
	std::string errMsg;
	auto r = pragma::scripting::lua::protected_call(l, 1, 1, &errMsg);
	if(r == Lua::StatusCode::Ok) {
		Lua::GetField(l, -1, "start");
		r = pragma::scripting::lua::protected_call(l, 0, 0, &errMsg);
		Lua::Pop(l, 1); // Pop return value of "require" from stack
	}
	if(r != Lua::StatusCode::Ok)
		Con::cwar << "Unable to enable remote debugging:\n" << errMsg << Con::endl;
}
