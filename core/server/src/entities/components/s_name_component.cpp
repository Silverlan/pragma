#include "stdafx_server.h"
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SNameComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->WriteString(GetName());
}
void SNameComponent::SetName(std::string name)
{
	BaseNameComponent::SetName(name);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->WriteString(name);
	server->BroadcastTCP("ent_setname",p);
}
luabind::object SNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SNameComponentHandleWrapper>(l);}
