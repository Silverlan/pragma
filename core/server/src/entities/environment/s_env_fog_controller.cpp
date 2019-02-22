#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_fog_controller.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_fog_controller,EnvFogController);

luabind::object SFogControllerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFogControllerComponentHandleWrapper>(l);}
void SFogControllerComponent::SetFogStart(float start)
{
	BaseEnvFogControllerComponent::SetFogStart(start);
	NetPacket p;
	nwm::write_entity(p,&GetEntity());
	p->Write<float>(m_kvFogStart);
	server->BroadcastTCP("env_fogcon_setstartdist",p);
}
void SFogControllerComponent::SetFogEnd(float end)
{
	BaseEnvFogControllerComponent::SetFogEnd(end);
	NetPacket p;
	nwm::write_entity(p,&GetEntity());
	p->Write<float>(m_kvFogEnd);
	server->BroadcastTCP("env_fogcon_setenddist",p);
}
void SFogControllerComponent::SetMaxDensity(float density)
{
	BaseEnvFogControllerComponent::SetMaxDensity(density);
	NetPacket p;
	nwm::write_entity(p,&GetEntity());
	p->Write<float>(m_kvMaxDensity);
	server->BroadcastTCP("env_fogcon_setmaxdensity",p);
}
void SFogControllerComponent::SetFogType(unsigned char type)
{
	BaseEnvFogControllerComponent::SetFogType(type);
}
void SFogControllerComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<unsigned char>(m_kvFogType);
	packet->Write<float>(m_kvFogStart);
	packet->Write<float>(m_kvFogEnd);
	packet->Write<float>(m_kvMaxDensity);
}

///////////////

void EnvFogController::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFogControllerComponent>();
}
