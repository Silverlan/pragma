// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.effects.sprite;

using namespace pragma;

void SSpriteComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_spritePath);
	packet->Write<float>(m_size);
	packet->Write<float>(m_bloomScale);
	packet->Write<Color>(m_color);
	packet->Write<uint32_t>(m_particleRenderMode);
	packet->Write<float>(m_tFadeIn);
	packet->Write<float>(m_tFadeOut);
}

void SSpriteComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvSprite::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSpriteComponent>();
}
