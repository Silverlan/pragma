/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/effects/s_env_sprite.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sprite, EnvSprite);

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
