// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convars.h>
#include <sharedutils/util.h>
#ifdef _WIN32
#include <Mmsystem.h>
#endif

module pragma.shared;

import :engine;

std::string Engine::GetDate(const std::string &format) { return util::get_date_time(format); }

uint64_t Engine::GetTickCount() const { return CUInt64(m_ctTick.GetTime()); }
double Engine::GetTickTime() const { return CDouble(m_ctTick()); }
UInt32 Engine::GetTickRate() const { return m_tickRate; }
void Engine::SetTickRate(UInt32 tickRate)
{
	assert(tickRate != 0);
	if(tickRate == 0) {
		Con::cwar << "Invalid tick rate '" << tickRate << "'. Clamping to '1'..." << Con::endl;
		tickRate = 1;
	}
	m_tickRate = tickRate;
}
bool Engine::IsRunning() { return umath::is_flag_set(m_stateFlags, StateFlags::Running); }
bool Engine::IsGameActive() { return GetServerNetworkState()->IsGameActive(); }
bool Engine::IsServerOnly() { return true; }
