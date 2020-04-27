/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/trigger/s_trigger_push.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_push,TriggerPush);

void STriggerPushComponent::Initialize()
{
	BaseTriggerPushComponent::Initialize();

	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnThink(static_cast<CEOnTick&>(evData.get()).deltaTime);
	});

	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();
}

///////

luabind::object STriggerPushComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STriggerPushComponentHandleWrapper>(l);}

void TriggerPush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerPushComponent>();
}
