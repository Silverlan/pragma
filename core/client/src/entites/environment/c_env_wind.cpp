#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_wind.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_wind,CEnvWind);

void CWindComponent::ReceiveData(NetPacket &packet)
{
	auto windForce = packet->Read<Vector3>();
	SetWindForce(windForce);
}
luabind::object CWindComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWindComponentHandleWrapper>(l);}

////////

void CEnvWind::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWindComponent>();
}
