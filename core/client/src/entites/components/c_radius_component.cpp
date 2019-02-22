#include "stdafx_client.h"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void CRadiusComponent::ReceiveData(NetPacket &packet)
{
	SetRadius(packet->Read<float>());
}
luabind::object CRadiusComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CRadiusComponentHandleWrapper>(l);}
Bool CRadiusComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetRadius)
		SetRadius(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
