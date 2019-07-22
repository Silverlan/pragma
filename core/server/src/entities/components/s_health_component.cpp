#include "stdafx_server.h"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SHealthComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
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
	server->SendPacket("ent_sethealth",p,pragma::networking::Protocol::SlowReliable);
}
luabind::object SHealthComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SHealthComponentHandleWrapper>(l);}
