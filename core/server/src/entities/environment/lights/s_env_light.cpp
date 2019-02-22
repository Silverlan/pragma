#include "stdafx_server.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/shared_spawnflags.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

extern ServerState *server;

void SLightComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<pragma::BaseEnvLightComponent::ShadowType>(GetShadowType());
	packet->Write<float>(m_falloffExponent);
}

void SLightComponent::SetShadowType(ShadowType type)
{
	BaseEnvLightComponent::SetShadowType(type);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write(type);
	ent.SendNetEventTCP(m_netEvSetShadowType,p);
}

void SLightComponent::SetFalloffExponent(float falloffExponent)
{
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write<float>(falloffExponent);
	ent.SendNetEventTCP(m_netEvSetFalloffExponent,p);
}

luabind::object SLightComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SLightComponentHandleWrapper>(l);}

