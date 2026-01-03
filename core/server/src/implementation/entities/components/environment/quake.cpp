// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.quake;

import :server_state;

using namespace pragma;

void SQuakeComponent::Initialize() { BaseEnvQuakeComponent::Initialize(); }

void SQuakeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<UInt32>(m_quakeFlags);
	packet->Write<Float>(GetFrequency());
	packet->Write<Float>(GetAmplitude());
	packet->Write<Float>(GetRadius());
	packet->Write<Float>(GetDuration());
	packet->Write<Float>(GetFadeInDuration());
	packet->Write<Float>(GetFadeOutDuration());
}

void SQuakeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvQuake::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SQuakeComponent>();
}
