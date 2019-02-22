#include "stdafx_client.h"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void CColorComponent::ReceiveData(NetPacket &packet)
{
	*m_color = packet->Read<Color>();
}
luabind::object CColorComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CColorComponentHandleWrapper>(l);}
Bool CColorComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetColor)
		SetColor(packet->Read<Color>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
