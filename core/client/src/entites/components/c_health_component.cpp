#include "stdafx_client.h"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

luabind::object CHealthComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CHealthComponentHandleWrapper>(l);}
void CHealthComponent::ReceiveData(NetPacket &packet)
{
	*m_health = packet->Read<unsigned short>();
}
