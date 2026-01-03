// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.lights.directional;

import :entities;
import :entities.components.lights.base;

using namespace pragma;

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
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetAmbientColor, p, networking::Protocol::SlowReliable);
}

void SLightDirectionalComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
