#include "stdafx_shared.h"
#include "pragma/networkdefinitions.h"
#include <pragma/engine.h>
#include "pragma/lua/libraries/ltimer.h"
#include <pragma/game/game.h>
#include "pragma/lua/ldefinitions.h"
#include "luasystem.h"

Timer::Timer()
	: m_bRemove(false),m_next(0),m_bRunning(false),m_bIsValid(true),m_callback(),
	m_function(-1)
{}

Timer::Timer(float delay,unsigned int reps,int function,TimerType timetype)
	: Timer()
{
	m_delay = delay;
	m_reps = reps;
	m_function = function;
	m_timeType = timetype;
}

Timer::Timer(float delay,unsigned int reps,const CallbackHandle &hCallback,TimerType timetype)
	: Timer()
{
	m_delay = delay;
	m_reps = reps;
	m_callback = hCallback;
	m_timeType = timetype;
}

Timer::~Timer()
{
	for(auto i=m_handles.size() -1;i!=size_t(-1);i--)
	{
		std::shared_ptr<TimerHandle> pTimer = m_handles[i];
		TimerHandle *hTimer = pTimer.get();
		if(hTimer != NULL)
			hTimer->m_timer = NULL;
	}
}

double Timer::GetCurTime(Game *game)
{
	switch(m_timeType)
	{
	case TimerType::CurTime:
		return game->CurTime();
	case TimerType::RealTime:
		return game->RealTime();
	}
	return game->ServerTime();
}

double Timer::GetDeltaTime(Game *game)
{
	switch(m_timeType)
	{
	case TimerType::CurTime:
		return game->DeltaTickTime();
	case TimerType::RealTime:
		return game->DeltaRealTime();
	}
	return game->DeltaTickTime();
}

void Timer::Update(Game *game)
{
	if(!m_bRunning)
		return;
	m_next -= static_cast<float>(GetDeltaTime(game));
	if(m_next <= 0)
	{
		Call(game);
		if(m_reps > 0)
		{
			m_reps--;
			if(m_reps == 0)
				Remove(game);
			else
				Reset();
		}
		else
			Reset();
	}
}

void Timer::Call(Game *game)
{
	if(m_function != -1)
	{
		game->ProtectedLuaCall([this](lua_State *l) {
			lua_rawgeti(l,LUA_REGISTRYINDEX,m_function);
			std::shared_ptr<TimerHandle> hTimer = CreateHandle();
			luabind::object(l,hTimer).push(l);
			return Lua::StatusCode::Ok;
		},0);
	}
	m_callback();
}

void Timer::SetCall(Game *game,int function)
{
	if(m_function != -1)
	{
		lua_removereference(game->GetLuaState(),m_function);
		m_function = -1;
	}
	m_function = function;
	m_callback = CallbackHandle();
}

void Timer::SetCall(Game *game,const CallbackHandle &hCallback)
{
	SetCall(game,-1);
	m_callback = hCallback;
}

void Timer::Reset()
{
	m_next = m_delay +m_next;
}

void Timer::Start(Game*)
{
	if(m_bRunning)
		return;
	if(m_next == 0.f)
		Reset();
	m_bRunning = true;
}

void Timer::Pause()
{
	m_bRunning = false;
}

void Timer::Stop()
{
	m_bRunning = false;
	m_next = 0.f;
}

void Timer::Remove(Game *game)
{
	m_bIsValid = false;
	if(m_function != -1)
	{
		luaL_unref(game->GetLuaState(),LUA_REGISTRYINDEX,m_function);
		m_function = -1;
	}
}

bool Timer::IsValid() {return m_bIsValid;}
bool Timer::IsRunning() {return m_bRunning;}

bool Timer::IsPaused() {return (!m_bRunning && m_next != 0.f) ? true : false;}

std::shared_ptr<TimerHandle> Timer::CreateHandle()
{
	std::shared_ptr<TimerHandle> pTimer(new TimerHandle(this));
	m_handles.push_back(pTimer);
	return pTimer;
}

void Timer::InvalidateHandle(TimerHandle *hTimer)
{
	for(auto i=m_handles.size() -1;i!=size_t(-1);i--)
	{
		if(m_handles[i].get() == hTimer)
		{
			hTimer->m_timer = NULL;
			m_handles.erase(m_handles.begin() +i);
			break;
		}
	}
}

float Timer::GetTimeLeft()
{
	if(m_reps == 0)
		return 0;
	return std::max(m_next,0.f) +(m_reps -1) *m_delay;
}
void Timer::SetTimeInterval(float time)
{
	float delayOld = m_delay;
	m_delay = time;
	if(!IsRunning())
		return;
	float tDelta = time -delayOld;
	m_next += tDelta;
}
float Timer::GetTimeInterval() {return m_delay;}
unsigned int Timer::GetRepetitionsLeft() {return m_reps;}
void Timer::SetRepetitions(unsigned int rep) {m_reps = rep;}
/*
TimerRandom::TimerRandom(float minInterval,float maxInterval,unsigned int reps,int function,unsigned char timetype)
	: Timer(minInterval,reps,function,timetype)
{
	m_minInterval = minInterval;
	m_maxInterval = maxInterval;
}

void TimerRandom::Reset()
{
	m_next = Math::Random(m_minInterval,m_maxInterval) +m_next;
}
*/
/////////////////////////////

extern DLLENGINE Engine *engine;
Timer *Game::CreateTimer(float delay,int reps,int fcLua,TimerType timeType)
{
	m_timers.push_back(std::make_unique<Timer>(delay,reps,fcLua,timeType));
	return m_timers.back().get();
}

Timer *Game::CreateTimer(float delay,int reps,const CallbackHandle &hCallback,TimerType timeType)
{
	m_timers.push_back(std::make_unique<Timer>(delay,reps,hCallback,timeType));
	return m_timers.back().get();
}

void Game::ClearTimers()
{
	m_timers.clear();
}

void Game::UpdateTimers()
{
	for(auto i=decltype(m_timers.size()){0u};i<m_timers.size();++i)
	{
		auto &timer = *m_timers.at(i);
		if(timer.IsValid() == false)
		{
			m_timers.erase(m_timers.begin() +i);
			continue;
		}
		timer.Update(this);
	}
}

/////////////////////////////

DLLNETWORK void Lua_Timer_Start(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	NetworkState *state = engine->GetNetworkState(l);
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->Start(state->GetGameState());
}

DLLNETWORK void Lua_Timer_Stop(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->Stop();
}

DLLNETWORK void Lua_Timer_Pause(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->Pause();
}

DLLNETWORK void Lua_Timer_Remove(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	NetworkState *state = engine->GetNetworkState(l);
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->Remove(state->GetGameState());
}

DLLNETWORK void Lua_Timer_IsValid(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_pushboolean(l,(pTimer.get() != NULL && pTimer.get()->IsValid()) ? true : false);
}

DLLNETWORK void Lua_Timer_GetTimeLeft(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	Lua::PushNumber(l,hTimer->GetTimer()->GetTimeLeft());
}

DLLNETWORK void Lua_Timer_GetTimeInterval(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	Lua::PushNumber(l,hTimer->GetTimer()->GetTimeInterval());
}

DLLNETWORK void Lua_Timer_SetTimeInterval(lua_State *l,std::shared_ptr<TimerHandle> pTimer,float time)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->SetTimeInterval(time);
}

DLLNETWORK void Lua_Timer_GetRepetitionsLeft(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	Lua::PushInt(l,hTimer->GetTimer()->GetRepetitionsLeft());
}

DLLNETWORK void Lua_Timer_SetRepetitions(lua_State *l,std::shared_ptr<TimerHandle> pTimer,unsigned int reps)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->SetRepetitions(reps);
}

DLLNETWORK void Lua_Timer_IsRunning(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	lua_pushboolean(l,hTimer->GetTimer()->IsRunning());
}

DLLNETWORK void Lua_Timer_IsPaused(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	lua_pushboolean(l,hTimer->GetTimer()->IsPaused());
}

DLLNETWORK void Lua_Timer_Call(lua_State *l,std::shared_ptr<TimerHandle> pTimer)
{
	NetworkState *state = engine->GetNetworkState(l);
	lua_checktimer(l,pTimer);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->Call(state->GetGameState());
}

DLLNETWORK void Lua_Timer_SetCall(lua_State *l,std::shared_ptr<TimerHandle> pTimer,luabind::object o)
{
	lua_checktimer(l,pTimer);
	luaL_checkfunction(l,2);
	int fc = lua_createreference(l,2);
	NetworkState *state = engine->GetNetworkState(l);
	TimerHandle *hTimer = pTimer.get();
	hTimer->GetTimer()->SetCall(state->GetGameState(),fc);
}

/////////////////////////////

int Lua::time::create_timer(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	float delay = static_cast<float>(Lua::CheckNumber(l,1));
	int reps = static_cast<int>(Lua::CheckInt(l,2));
	luaL_checkfunction(l,3);
	int fc = lua_createreference(l,3);
	TimerType timeType;
	if(!lua_isnoneornil(l,4))
		timeType = static_cast<TimerType>(Lua::CheckInt(l,4));
	else timeType = TimerType::CurTime;
	Timer *timer = game->CreateTimer(delay,reps,fc,timeType);
	std::shared_ptr<TimerHandle> hTimer = timer->CreateHandle();
	luabind::object(l,hTimer).push(l);
	return 1;
}

int Lua::time::create_simple_timer(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	float delay = static_cast<float>(Lua::CheckNumber(l,1));
	luaL_checkfunction(l,2);
	int fc = lua_createreference(l,2);
	TimerType timeType;
	if(!lua_isnoneornil(l,3))
		timeType = static_cast<TimerType>(Lua::CheckInt(l,3));
	else timeType = TimerType::CurTime;
	Timer *timer = game->CreateTimer(delay,1,fc,timeType);
	timer->Start(game);
	std::shared_ptr<TimerHandle> hTimer = timer->CreateHandle();
	luabind::object(l,hTimer).push(l);
	return 1;
}
