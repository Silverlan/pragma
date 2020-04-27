/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/engine.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/console/cvar.h"
#include <pragma/console/convars.h>
#include <sharedutils/util.h>
#ifdef _WIN32
	#include <Mmsystem.h>
#endif
std::string Engine::GetDate(const std::string &format)
{
	return util::get_date_time(format);
}

uint64_t Engine::GetTickCount() const {return CUInt64(m_ctTick.GetTime());}
double Engine::GetTickTime() const {return CDouble(m_ctTick());}
UInt32 Engine::GetTickRate() const {return m_tickRate;}
void Engine::SetTickRate(UInt32 tickRate)
{
	assert(tickRate != 0);
	if(tickRate == 0)
	{
		Con::cwar<<"WARNING: Invalid tick rate '"<<tickRate<<"'. Clamping to '1'..."<<Con::endl;
		tickRate = 1;
	}
	m_tickRate = tickRate;
}
bool Engine::IsRunning() {return m_bRunning;}
bool Engine::IsGameActive() {return GetServerState()->IsGameActive();}
bool Engine::IsServerOnly() {return true;}