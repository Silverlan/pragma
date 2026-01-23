// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.effects.smoke_trail;

import :game;
import :server_state;

using namespace pragma;

void SSmokeTrailComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_speed);
	packet->Write<float>(m_distance);
	packet->Write<float>(m_minSpriteSize);
	packet->Write<float>(m_maxSpriteSize);
	packet->WriteString(m_material);
}

void SSmokeTrailComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvSmokeTrail::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSmokeTrailComponent>();
}
