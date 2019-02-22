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

const long long Engine::GetTickCount() {return CUInt64(m_ctTick.GetTime());}
double Engine::GetTickTime() {return CDouble(m_ctTick());}
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