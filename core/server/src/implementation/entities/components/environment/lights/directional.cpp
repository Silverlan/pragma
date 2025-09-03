// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>
#include "pragma/entities/s_baseentity.h"

module pragma.server.entities.components.lights.directional;

import pragma.server.entities.components.lights.base;

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_environment, EnvLightDirectional);

void SLightDirectionalComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
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
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetAmbientColor, p, pragma::networking::Protocol::SlowReliable);
}

void SLightDirectionalComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLightDirectionalComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SLightComponent))
		static_cast<SLightComponent &>(component).SetLight(*this);
}

//////////////

void EnvLightDirectional::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightComponent>();
	AddComponent<SLightDirectionalComponent>();
}
