#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_timescale.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SEnvTimescaleComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(m_kvTimescale);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
}

LINK_ENTITY_TO_CLASS(env_timescale,EnvTimescale);

luabind::object SEnvTimescaleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STimeScaleComponentHandleWrapper>(l);}

void EnvTimescale::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SEnvTimescaleComponent>();
}
