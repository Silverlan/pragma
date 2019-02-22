#include "stdafx_client.h"
#include "pragma/entities/components/c_time_scale_component.hpp"

using namespace pragma;

luabind::object CTimeScaleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CTimeScaleComponentHandleWrapper>(l);}
Bool CTimeScaleComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetTimeScale)
	{
		SetTimeScale(packet->Read<float>());
		return true;
	}
	return false;
}
