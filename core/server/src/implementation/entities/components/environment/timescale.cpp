// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.timescale;

using namespace pragma;

void SEnvTimescaleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvTimescale);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
}

void SEnvTimescaleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvTimescale::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SEnvTimescaleComponent>();
}
