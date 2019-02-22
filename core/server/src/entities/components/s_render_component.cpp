#include "stdafx_server.h"
#include "pragma/entities/components/s_render_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

extern DLLSERVER ServerState *server;

void SRenderComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
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
	server->BroadcastTCP("ent_setunlit",p);
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
	server->BroadcastTCP("ent_setcastshadows",p);
}
