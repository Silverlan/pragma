#include "stdafx_server.h"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SOwnableComponent::Initialize()
{
	BaseOwnableComponent::Initialize();
}
luabind::object SOwnableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SOwnableComponentHandleWrapper>(l);}

void SOwnableComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	nwm::write_entity(packet,*m_owner);
}

void SOwnableComponent::SetOwner(BaseEntity *owner)
{
	BaseOwnableComponent::SetOwner(owner);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	nwm::write_entity(p,*m_owner);
	ent.SendNetEventTCP(m_netEvSetOwner,p);
}
