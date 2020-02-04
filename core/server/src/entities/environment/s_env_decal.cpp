#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_decal.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/environment/env_decal.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_decal,EnvDecal);

luabind::object SDecalComponent::InitializeLuaObject(lua_State *l) {return BaseEnvDecalComponent::InitializeLuaObject<SDecalComponentHandleWrapper>(l);}

void SDecalComponent::Initialize()
{
	BaseEnvDecalComponent::Initialize();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	ent.SetSynchronized(false);
}

void SDecalComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_material);
	packet->Write<float>(m_size);
	packet->Write<bool>(m_startDisabled);
}

void EnvDecal::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDecalComponent>();
}
