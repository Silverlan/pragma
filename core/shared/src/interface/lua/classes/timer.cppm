// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.timer;

export import :util.timer;

export {
	namespace Lua {
		namespace time {
			std::shared_ptr<TimerHandle> create_timer(lua::State *l, float delay, int32_t repetitions, LuaFunctionObject fc, TimerType timerType);
			std::shared_ptr<TimerHandle> create_timer(lua::State *l, float delay, int32_t repetitions, LuaFunctionObject fc);
			std::shared_ptr<TimerHandle> create_simple_timer(lua::State *l, float delay, LuaFunctionObject fc, TimerType timerType);
			std::shared_ptr<TimerHandle> create_simple_timer(lua::State *l, float delay, LuaFunctionObject fc);
		};
	};

	DLLNETWORK void Lua_Timer_Start(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Stop(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Pause(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Remove(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_IsValid(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_GetTimeLeft(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_GetTimeInterval(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetTimeInterval(lua::State *l, TimerHandle &timer, float time);
	DLLNETWORK void Lua_Timer_GetRepetitionsLeft(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetRepetitions(lua::State *l, TimerHandle &timer, unsigned int reps);
	DLLNETWORK void Lua_Timer_IsRunning(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_IsPaused(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Call(lua::State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetCall(lua::State *l, TimerHandle &timer, LuaFunctionObject o);
};
