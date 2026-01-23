// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.lights.base;

import :entities;
import :server_state;

using namespace pragma;

void SLightComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<ShadowType>(GetShadowType());
	packet->Write<float>(m_falloffExponent);
	packet->Write<LightFlags>(m_lightFlags);
	packet->Write<float>(GetLightIntensity());
	packet->Write<LightIntensityType>(GetLightIntensityType());
}

void SLightComponent::SetShadowType(ShadowType type)
{
	BaseEnvLightComponent::SetShadowType(type);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write(type);
	ent.SendNetEvent(m_netEvSetShadowType, p, networking::Protocol::SlowReliable);
}

void SLightComponent::SetFalloffExponent(float falloffExponent)
{
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write<float>(falloffExponent);
	ent.SendNetEvent(m_netEvSetFalloffExponent, p, networking::Protocol::SlowReliable);
}

void SLightComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
