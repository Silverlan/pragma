// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#include <Mmsystem.h>
#endif

module pragma.shared;

import :engine;

std::string pragma::Engine::GetDate(const std::string &format) { return util::get_date_time(format); }

uint64_t pragma::Engine::GetTickCount() const { return CUInt64(m_ctTick.GetTime()); }
double pragma::Engine::GetTickTime() const { return CDouble(m_ctTick()); }
UInt32 pragma::Engine::GetTickRate() const { return m_tickRate; }
void pragma::Engine::SetTickRate(UInt32 tickRate)
{
	assert(tickRate != 0);
	if(tickRate == 0) {
		Con::CWAR << "Invalid tick rate '" << tickRate << "'. Clamping to '1'..." << Con::endl;
		tickRate = 1;
	}
	m_tickRate = tickRate;
}
bool pragma::Engine::IsRunning() { return math::is_flag_set(m_stateFlags, StateFlags::Running); }
bool pragma::Engine::IsGameActive() { return GetServerNetworkState()->IsGameActive(); }
bool pragma::Engine::IsServerOnly() { return true; }
