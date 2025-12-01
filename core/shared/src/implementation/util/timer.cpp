// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.timer;

#undef max

Timer::Timer() : m_bRemove(false), m_next(0), m_bRunning(false), m_bIsValid(true), m_callback() {}

Timer::Timer(float delay, unsigned int reps, LuaFunctionObject luaFunction, TimerType timetype) : Timer()
{
	m_delay = delay;
	m_reps = reps;
	m_luaFunction = luaFunction;
	m_timeType = timetype;
}

Timer::Timer(float delay, unsigned int reps, const CallbackHandle &hCallback, TimerType timetype) : Timer()
{
	m_delay = delay;
	m_reps = reps;
	m_callback = hCallback;
	m_timeType = timetype;
}

Timer::~Timer()
{
	for(auto i = m_handles.size() - 1; i != size_t(-1); i--) {
		std::shared_ptr<TimerHandle> pTimer = m_handles[i];
		TimerHandle *hTimer = pTimer.get();
		if(hTimer != nullptr)
			hTimer->m_timer = nullptr;
	}
}

double Timer::GetCurTime(pragma::Game *game)
{
	switch(m_timeType) {
	case TimerType::CurTime:
		return game->CurTime();
	case TimerType::RealTime:
		return game->RealTime();
	}
	return game->ServerTime();
}

double Timer::GetDeltaTime(pragma::Game *game)
{
	switch(m_timeType) {
	case TimerType::CurTime:
		return game->DeltaTickTime();
	case TimerType::RealTime:
		return game->DeltaRealTime();
	}
	return game->DeltaTickTime();
}

void Timer::Update(pragma::Game *game)
{
	if(!m_bRunning)
		return;
	m_next -= static_cast<float>(GetDeltaTime(game));
	if(m_next <= 0) {
		Call(game);
		if(m_reps > 0) {
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

void Timer::Call(pragma::Game *game)
{
	if(m_luaFunction.is_valid()) {
		game->ProtectedLuaCall(
		  [this](lua::State *l) {
			  m_luaFunction.push(l);
			  Lua::Push(l, CreateHandle());
			  return Lua::StatusCode::Ok;
		  },
		  0);
	}
	m_callback();
}

void Timer::SetCall(pragma::Game *game, LuaFunctionObject luaFunction)
{
	m_luaFunction = luaFunction;
	m_callback = CallbackHandle();
}

void Timer::SetCall(pragma::Game *game, const CallbackHandle &hCallback)
{
	m_luaFunction = {};
	m_callback = hCallback;
}

void Timer::Reset() { m_next = m_delay + m_next; }

void Timer::Start(pragma::Game *)
{
	if(m_bRunning)
		return;
	if(m_next == 0.f)
		Reset();
	m_bRunning = true;
}

void Timer::Pause() { m_bRunning = false; }

void Timer::Stop()
{
	m_bRunning = false;
	m_next = 0.f;
}

void Timer::Remove(pragma::Game *game)
{
	m_bIsValid = false;
	m_luaFunction = {};
}

bool Timer::IsValid() { return m_bIsValid; }
bool Timer::IsRunning() { return m_bRunning; }

bool Timer::IsPaused() { return (!m_bRunning && m_next != 0.f) ? true : false; }

std::shared_ptr<TimerHandle> Timer::CreateHandle()
{
	std::shared_ptr<TimerHandle> pTimer(new TimerHandle(this));
	m_handles.push_back(pTimer);
	return pTimer;
}

void Timer::InvalidateHandle(TimerHandle *hTimer)
{
	for(auto i = m_handles.size() - 1; i != size_t(-1); i--) {
		if(m_handles[i].get() == hTimer) {
			hTimer->m_timer = nullptr;
			m_handles.erase(m_handles.begin() + i);
			break;
		}
	}
}

float Timer::GetTimeLeft()
{
	if(m_reps == 0)
		return 0;
	return std::max(m_next, 0.f) + (m_reps - 1) * m_delay;
}
void Timer::SetTimeInterval(float time)
{
	float delayOld = m_delay;
	m_delay = time;
	if(!IsRunning())
		return;
	float tDelta = time - delayOld;
	m_next += tDelta;
}
float Timer::GetTimeInterval() { return m_delay; }
unsigned int Timer::GetRepetitionsLeft() { return m_reps; }
void Timer::SetRepetitions(unsigned int rep) { m_reps = rep; }
