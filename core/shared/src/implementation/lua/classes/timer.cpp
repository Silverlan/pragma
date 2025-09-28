// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "luasystem.h"

module pragma.shared;

import :scripting.lua.classes.timer;

DLLNETWORK void Lua_Timer_Start(lua_State *l, TimerHandle &timer)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	lua_checktimer(l, timer);
	timer.GetTimer()->Start(state->GetGameState());
}

DLLNETWORK void Lua_Timer_Stop(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	timer.GetTimer()->Stop();
}

DLLNETWORK void Lua_Timer_Pause(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	timer.GetTimer()->Pause();
}

DLLNETWORK void Lua_Timer_Remove(lua_State *l, TimerHandle &timer)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	lua_checktimer(l, timer);
	timer.GetTimer()->Remove(state->GetGameState());
}

DLLNETWORK void Lua_Timer_IsValid(lua_State *l, TimerHandle &timer) { lua_pushboolean(l, timer.IsValid()); }

DLLNETWORK void Lua_Timer_GetTimeLeft(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	Lua::PushNumber(l, timer.GetTimer()->GetTimeLeft());
}

DLLNETWORK void Lua_Timer_GetTimeInterval(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	Lua::PushNumber(l, timer.GetTimer()->GetTimeInterval());
}

DLLNETWORK void Lua_Timer_SetTimeInterval(lua_State *l, TimerHandle &timer, float time)
{
	lua_checktimer(l, timer);
	timer.GetTimer()->SetTimeInterval(time);
}

DLLNETWORK void Lua_Timer_GetRepetitionsLeft(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	Lua::PushInt(l, timer.GetTimer()->GetRepetitionsLeft());
}

DLLNETWORK void Lua_Timer_SetRepetitions(lua_State *l, TimerHandle &timer, unsigned int reps)
{
	lua_checktimer(l, timer);
	timer.GetTimer()->SetRepetitions(reps);
}

DLLNETWORK void Lua_Timer_IsRunning(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	lua_pushboolean(l, timer.GetTimer()->IsRunning());
}

DLLNETWORK void Lua_Timer_IsPaused(lua_State *l, TimerHandle &timer)
{
	lua_checktimer(l, timer);
	lua_pushboolean(l, timer.GetTimer()->IsPaused());
}

DLLNETWORK void Lua_Timer_Call(lua_State *l, TimerHandle &timer)
{
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	lua_checktimer(l, timer);
	timer.GetTimer()->Call(state->GetGameState());
}

DLLNETWORK void Lua_Timer_SetCall(lua_State *l, TimerHandle &timer, LuaFunctionObject o)
{
	lua_checktimer(l, timer);
	NetworkState *state = Engine::Get()->GetNetworkState(l);
	timer.GetTimer()->SetCall(state->GetGameState(), o);
}

/////////////////////////////

std::shared_ptr<TimerHandle> Lua::time::create_timer(lua_State *l, float delay, int32_t repetitions, LuaFunctionObject fc, TimerType timerType)
{
	Lua::CheckType(fc, Lua::Type::Function);
	auto *state = Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *timer = game->CreateTimer(delay, repetitions, fc, timerType);
	return timer->CreateHandle();
}
std::shared_ptr<TimerHandle> Lua::time::create_timer(lua_State *l, float delay, int32_t repetitions, LuaFunctionObject fc) { return create_timer(l, delay, repetitions, fc, TimerType::CurTime); }

std::shared_ptr<TimerHandle> Lua::time::create_simple_timer(lua_State *l, float delay, LuaFunctionObject fc, TimerType timerType)
{
	Lua::CheckType(fc, Lua::Type::Function);
	auto *state = Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *timer = game->CreateTimer(delay, 1, fc, timerType);
	timer->Start(game);
	return timer->CreateHandle();
}
std::shared_ptr<TimerHandle> Lua::time::create_simple_timer(lua_State *l, float delay, LuaFunctionObject fc) { return create_simple_timer(l, delay, fc, TimerType::CurTime); }
