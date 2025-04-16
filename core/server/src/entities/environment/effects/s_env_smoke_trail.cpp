/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/effects/s_env_smoke_trail.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/game/damageinfo.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_smoke_trail, EnvSmokeTrail);

extern ServerState *server;
extern SGame *s_game;

void SSmokeTrailComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_speed);
	packet->Write<float>(m_distance);
	packet->Write<float>(m_minSpriteSize);
	packet->Write<float>(m_maxSpriteSize);
	packet->WriteString(m_material);
}

void SSmokeTrailComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvSmokeTrail::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSmokeTrailComponent>();
}
