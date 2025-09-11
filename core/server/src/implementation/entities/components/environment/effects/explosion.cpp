// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/game/damageinfo.h>
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.effects.explosion;

import pragma.server.server_state;

using namespace pragma;

void SExplosionComponent::Explode()
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		ServerState::Get()->SendPacket("envexplosion_explode", p, pragma::networking::Protocol::SlowReliable);
	}
	BaseEnvExplosionComponent::Explode();
}

void SExplosionComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvExplosion::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SExplosionComponent>();
}
