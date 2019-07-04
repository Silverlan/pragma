#include "stdafx_server.h"
#include "pragma/entities/components/s_render_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SRenderComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->Write<decltype(m_renderFlags)>(m_renderFlags);
}
luabind::object SRenderComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SRenderComponentHandleWrapper>(l);}
void SRenderComponent::SetUnlit(bool b)
{
	BaseRenderComponent::SetUnlit(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<bool>(b);
	server->SendPacket("ent_setunlit",p,pragma::networking::Protocol::SlowReliable);
}
void SRenderComponent::SetCastShadows(bool b)
{
	BaseRenderComponent::SetCastShadows(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<bool>(b);
	server->SendPacket("ent_setcastshadows",p,pragma::networking::Protocol::SlowReliable);
}
