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
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(bot,Bot);

luabind::object SBotComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SBotComponentHandleWrapper>(l);}

void Bot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBotComponent>();
}
