#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_timescale.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_timescale,CEnvTimescale);

void CEnvTimescaleComponent::ReceiveData(NetPacket &packet)
{
	m_kvTimescale = packet->Read<float>();
	m_kvInnerRadius = packet->Read<float>();
	m_kvOuterRadius = packet->Read<float>();
}
luabind::object CEnvTimescaleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CEnvTimescaleComponentHandleWrapper>(l);}

//////////

void CEnvTimescale::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CEnvTimescaleComponent>();
}
