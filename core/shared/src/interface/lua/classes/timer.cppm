// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <memory>

export module pragma.shared:scripting.lua.classes.timer;

export import :util.timer;

export {
	namespace Lua {
		namespace time {
			std::shared_ptr<TimerHandle> create_timer(lua_State *l, float delay, int32_t repetitions, LuaFunctionObject fc, TimerType timerType);
			std::shared_ptr<TimerHandle> create_timer(lua_State *l, float delay, int32_t repetitions, LuaFunctionObject fc);
			std::shared_ptr<TimerHandle> create_simple_timer(lua_State *l, float delay, LuaFunctionObject fc, TimerType timerType);
			std::shared_ptr<TimerHandle> create_simple_timer(lua_State *l, float delay, LuaFunctionObject fc);
		};
	};

	DLLNETWORK void Lua_Timer_Start(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Stop(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Pause(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Remove(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_IsValid(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_GetTimeLeft(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_GetTimeInterval(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetTimeInterval(lua_State *l, TimerHandle &timer, float time);
	DLLNETWORK void Lua_Timer_GetRepetitionsLeft(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetRepetitions(lua_State *l, TimerHandle &timer, unsigned int reps);
	DLLNETWORK void Lua_Timer_IsRunning(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_IsPaused(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_Call(lua_State *l, TimerHandle &timer);
	DLLNETWORK void Lua_Timer_SetCall(lua_State *l, TimerHandle &timer, LuaFunctionObject o);
};
