// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.lights.spot_vol;

import :entities;
import :networking.util;

using namespace pragma;

void SLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	static_cast<SBaseEntity &>(GetEntity()).SetSynchronized(false);
}
void SLightSpotVolComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_coneStartOffset);
	networking::write_unique_entity(packet, m_hSpotlightTarget.get());
}

void SLightSpotVolComponent::SetSpotlightTarget(ecs::BaseEntity &ent)
{
	BaseEnvLightSpotVolComponent::SetSpotlightTarget(ent);
	NetPacket p {};
	networking::write_entity(p, &ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSpotlightTarget, p, networking::Protocol::SlowReliable);
}

void SLightSpotVolComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvLightSpotVol::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightSpotVolComponent>();
}
