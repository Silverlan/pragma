/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/ai/navsystem.h>
#include <pragma/model/animation/activities.h>
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/s_disposition.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(bot, Bot);

void SBotComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void Bot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBotComponent>();
}
