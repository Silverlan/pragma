/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LTIMER_H__
#define __LTIMER_H__
#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>

class TimerHandle;
namespace Lua
{
	namespace time
	{
		std::shared_ptr<TimerHandle> create_timer(lua_State *l,float delay,int32_t repetitions,luabind::function<> fc,TimerType timerType=TimerType::CurTime);
		std::shared_ptr<TimerHandle> create_simple_timer(lua_State *l,float delay,luabind::function<> fc,TimerType timerType=TimerType::CurTime);
	};
};

#include "pragma/util/timertypes.h"

class Game;
class TimerHandle;
class DLLNETWORK Timer
{
private:
	TimerType m_timeType;
	float m_delay;
	unsigned int m_reps;
	std::optional<luabind::function<>> m_luaFunction {};
	CallbackHandle m_callback;
	double m_start;
	bool m_bRemove;
	bool m_bRunning;
	bool m_bIsValid;
	std::vector<std::shared_ptr<TimerHandle>> m_handles;

	double GetCurTime(Game *game);
	double GetDeltaTime(Game *game);
protected:
	float m_next;
	virtual void Reset();
	Timer();
public:
	Timer(float delay,unsigned int reps,luabind::function<> luaFunction,TimerType timetype=TimerType::CurTime);
	Timer(float delay,unsigned int reps,const CallbackHandle &hCallback,TimerType timetype=TimerType::CurTime);
	~Timer();
	void Update(Game *game);
	void Start(Game *game);
	void Pause();
	void Stop();
	void Remove(Game *game);
	bool IsValid();
	bool IsRunning();
	bool IsPaused();
	void InvalidateHandle(TimerHandle *hTimer);
	float GetTimeLeft();
	void SetTimeInterval(float time);
	float GetTimeInterval();
	unsigned int GetRepetitionsLeft();
	void SetRepetitions(unsigned int rep);
	std::shared_ptr<TimerHandle> CreateHandle();
	void SetCall(Game *game,luabind::function<> luaFunction);
	void SetCall(Game *game,const CallbackHandle &hCallback);

	void Call(Game *game);
};

#include "pragma/util/timer_handle.h"

DLLNETWORK void Lua_Timer_Start(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_Stop(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_Pause(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_Remove(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_IsValid(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_GetTimeLeft(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_GetTimeInterval(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_SetTimeInterval(lua_State *l,TimerHandle &timer,float time);
DLLNETWORK void Lua_Timer_GetRepetitionsLeft(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_SetRepetitions(lua_State *l,TimerHandle &timer,unsigned int reps);
DLLNETWORK void Lua_Timer_IsRunning(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_IsPaused(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_Call(lua_State *l,TimerHandle &timer);
DLLNETWORK void Lua_Timer_SetCall(lua_State *l,TimerHandle &timer,luabind::function<> o);

#endif