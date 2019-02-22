#include "stdafx_client.h"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void CNameComponent::ReceiveData(NetPacket &packet)
{
	std::string name = packet->ReadString();
	SetName(name);
}
luabind::object CNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CNameComponentHandleWrapper>(l);}
