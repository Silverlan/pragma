#include "stdafx_client.h"
#include "pragma/entities/components/c_ownable_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void COwnableComponent::ReceiveData(NetPacket &packet)
{
	SetOwner(nwm::read_entity(packet));
}
luabind::object COwnableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<COwnableComponentHandleWrapper>(l);}

Bool COwnableComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetOwner)
	{
		SetOwner(nwm::read_entity(packet));
		return true;
	}
	return false;
}
