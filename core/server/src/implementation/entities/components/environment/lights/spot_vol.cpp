// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server.entities.components.lights.spot_vol;

import pragma.server.entities;
import pragma.server.networking.util;

using namespace pragma;

void SLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	static_cast<SBaseEntity &>(GetEntity()).SetSynchronized(false);
}
void SLightSpotVolComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_coneStartOffset);
	nwm::write_unique_entity(packet, m_hSpotlightTarget.get());
}

void SLightSpotVolComponent::SetSpotlightTarget(pragma::ecs::BaseEntity &ent)
{
	BaseEnvLightSpotVolComponent::SetSpotlightTarget(ent);
	NetPacket p {};
	nwm::write_entity(p, &ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSpotlightTarget, p, pragma::networking::Protocol::SlowReliable);
}

void SLightSpotVolComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvLightSpotVol::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightSpotVolComponent>();
}
