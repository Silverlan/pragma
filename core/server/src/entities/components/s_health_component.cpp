#include "stdafx_server.h"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SHealthComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<unsigned short>(GetHealth());
}
void SHealthComponent::SetHealth(unsigned short health)
{
	BaseHealthComponent::SetHealth(health);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<unsigned short>(health);
	server->BroadcastTCP("ent_sethealth",p);
}
luabind::object SHealthComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SHealthComponentHandleWrapper>(l);}
