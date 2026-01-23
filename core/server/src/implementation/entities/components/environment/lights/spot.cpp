// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.lights.spot;

import :entities;
import :entities.components.field_angle;
import :entities.components.lights.base;
import :server_state;

using namespace pragma;

void SLightSpotComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(*m_blendFraction);
	packet->Write<float>(*m_coneStartOffset);
}

void SLightSpotComponent::SetConeStartOffset(float offset)
{
	BaseEnvLightSpotComponent::SetConeStartOffset(offset);
	NetPacket p {};
	p->Write<float>(offset);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetConeStartOffset, p, networking::Protocol::SlowReliable);
}

void SLightSpotComponent::SetOuterConeAngle(float ang)
{
	BaseEnvLightSpotComponent::SetOuterConeAngle(ang);
	auto &ent = GetEntity();
	if(!ent.IsSpawned())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<float>(ang);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_LIGHT_SPOT_OUTERCUTOFF_ANGLE, p, networking::Protocol::SlowReliable);
}

void SLightSpotComponent::SetBlendFraction(float ang)
{
	BaseEnvLightSpotComponent::SetBlendFraction(ang);
	auto &ent = GetEntity();
	if(!ent.IsSpawned())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<float>(ang);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_LIGHT_SPOT_INNERCUTOFF_ANGLE, p, networking::Protocol::SlowReliable);
}

void SLightSpotComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLightSpotComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SLightComponent))
		static_cast<SLightComponent &>(component).SetLight(*this);
	else if(typeid(component) == typeid(SFieldAngleComponent))
		SetFieldAngleComponent(static_cast<SFieldAngleComponent &>(component));
}

///////////

void EnvLightSpot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightComponent>();
	AddComponent<SLightSpotComponent>();
}
