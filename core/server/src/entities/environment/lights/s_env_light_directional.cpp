#include "stdafx_server.h"
#include "pragma/entities/environment/lights/s_env_light_directional.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_environment,EnvLightDirectional);

void SLightDirectionalComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->Write<short>((*m_ambientColor)->r);
	packet->Write<short>((*m_ambientColor)->g);
	packet->Write<short>((*m_ambientColor)->b);
	packet->Write<short>((*m_ambientColor)->a);
	packet->Write<Float>(m_maxExposure);
}
void SLightDirectionalComponent::SetAmbientColor(const Color &color)
{
	BaseEnvLightDirectionalComponent::SetAmbientColor(color);
	NetPacket p;
	p->Write<short>(color.r);
	p->Write<short>(color.g);
	p->Write<short>(color.b);
	p->Write<short>(color.a);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEvent(m_netEvSetAmbientColor,p,pragma::networking::Protocol::SlowReliable);
}

luabind::object SLightDirectionalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SLightDirectionalComponentHandleWrapper>(l);}

//////////////

void EnvLightDirectional::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightComponent>("light");
	AddComponent<SLightDirectionalComponent>("light_directional");
}
