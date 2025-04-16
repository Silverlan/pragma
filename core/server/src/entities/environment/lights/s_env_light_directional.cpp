/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/lights/s_env_light_directional.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>

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
