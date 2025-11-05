// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;




#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
// required for beep
//#include <ncurses.h>
#elif _WIN32
#include <Windows.h>
#endif

module pragma.shared;

import :scripting.lua.libraries.debug;

//import pragma.scripting.lua;

int Lua::debug::collectgarbage(lua::State *l)
{
	// Calling twice on purpose: https://stackoverflow.com/a/28320364/2482983
	std::string err;
	Lua::RunString(l, "collectgarbage()", "internal", err);
	Lua::RunString(l, "collectgarbage()", "internal", err);
	return 0;
}
void Lua::debug::stackdump(lua::State *l)
{
	int top = lua::get_top(l);
	Con::cout << "Total in stack: " << top << Con::endl;
	for(int i = 1; i <= top; i++) {
		auto t = lua::type(l, i);
		switch(t) {
		case lua::Type::String:
			Con::cout << "string: " << lua::to_string(l, i) << Con::endl;
			break;
		case lua::Type::Boolean:
			Con::cout << "boolean: " << (lua::to_boolean(l, i) ? "true" : "false") << Con::endl;
			break;
		case lua::Type::Number:
			Con::cout << "number: " << lua::to_number(l, i) << Con::endl;
			break;
		default:
			Con::cout << lua::type_name(l, i) << Con::endl;
			break;
		}
	}
	if(top > 0)
		Con::cout << Con::endl;
}

void Lua::debug::beep(lua::State *l)
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

std::string Lua::debug::move_state_to_string(lua::State *l, pragma::BaseAIComponent::MoveResult v)
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

void Lua::debug::enable_remote_debugging(lua::State *l)
{
	if(get_extended_lua_modules_enabled() == false) {
		Con::cwar << "Unable to enable remote debugging: Game has to be started with -luaext launch parameter!" << Con::endl;
		return;
	}

	Lua::GetGlobal(l, "require");
	Lua::PushString(l, "modules/mobdebug"); // Note: This will disable jit!
	std::string errMsg;
	auto r = pragma::scripting::lua_core::protected_call(l, 1, 1, &errMsg);
	if(r == Lua::StatusCode::Ok) {
		Lua::GetField(l, -1, "start");
		r = pragma::scripting::lua_core::protected_call(l, 0, 0, &errMsg);
		Lua::Pop(l, 1); // Pop return value of "require" from stack
	}
	if(r != Lua::StatusCode::Ok)
		Con::cwar << "Unable to enable remote debugging:\n" << errMsg << Con::endl;
}
