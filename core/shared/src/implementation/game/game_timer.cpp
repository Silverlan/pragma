// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "luasystem.h"

module pragma.shared;

import :game.game;

Timer *Game::CreateTimer(float delay, int reps, LuaFunctionObject luaFunction, TimerType timeType)
{
	m_timers.push_back(std::make_unique<Timer>(delay, reps, luaFunction, timeType));
	return m_timers.back().get();
}

Timer *Game::CreateTimer(float delay, int reps, const CallbackHandle &hCallback, TimerType timeType)
{
	m_timers.push_back(std::make_unique<Timer>(delay, reps, hCallback, timeType));
	return m_timers.back().get();
}

void Game::ClearTimers() { m_timers.clear(); }

void Game::UpdateTimers()
{
	for(auto i = decltype(m_timers.size()) {0u}; i < m_timers.size(); ++i) {
		auto &timer = *m_timers.at(i);
		if(timer.IsValid() == false) {
			m_timers.erase(m_timers.begin() + i);
			continue;
		}
		timer.Update(this);
	}
}
