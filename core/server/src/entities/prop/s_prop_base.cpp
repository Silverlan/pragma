#include "stdafx_server.h"
#include "pragma/entities/prop/s_prop_base.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

using namespace pragma;

luabind::object PropComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<PropComponentHandleWrapper>(l);}

void PropComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(m_kvMaxVisibleDist);
	packet->Write<float>(m_kvMass);
}
