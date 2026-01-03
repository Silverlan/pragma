// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.skybox;

import :entities.base;

using namespace pragma;

void SSkyboxComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SSkyboxComponent::SetSkyAngles(const EulerAngles &ang)
{
	BaseSkyboxComponent::SetSkyAngles(ang);

	NetPacket p {};
	p->Write<EulerAngles>(ang);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetSkyAngles, p, networking::Protocol::SlowReliable);
}

void SSkyboxComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<EulerAngles>(m_skyAngles); }

void Skybox::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSkyboxComponent>();
}
