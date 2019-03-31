#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_wind.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <networkmanager/nwm_packet.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_wind,EnvWind);

void SWindComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<Vector3>(GetWindForce());
}

luabind::object SWindComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SWindComponentHandleWrapper>(l);}

void EnvWind::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SWindComponent>();
}
