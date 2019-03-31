#include "stdafx_server.h"
#include "pragma/entities/components/s_toggle_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

extern DLLSERVER SGame *s_game;

using namespace pragma;

void SToggleComponent::TurnOn()
{
	if(IsTurnedOn())
		return;
	BaseToggleComponent::TurnOn();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(true);
	ent.SendNetEventTCP(m_netEvToggleState,p);
}
void SToggleComponent::TurnOff()
{
	if(IsTurnedOn() == false)
		return;
	BaseToggleComponent::TurnOff();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(false);
	ent.SendNetEventTCP(m_netEvToggleState,p);
}
luabind::object SToggleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SToggleComponentHandleWrapper>(l);}
void SToggleComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<bool>(IsTurnedOn());
}

