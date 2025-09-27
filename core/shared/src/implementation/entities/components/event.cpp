// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_component_event.hpp"

module pragma.shared;

import :entities.components.events.event;

using namespace pragma;

CEOnEntityComponentAdded::CEOnEntityComponentAdded(BaseEntityComponent &component) : component(component) {}
void CEOnEntityComponentAdded::PushArguments(lua_State *l) { component.PushLuaObject(l); }

/////////////

CEOnMembersChanged::CEOnMembersChanged(BaseEntityComponent &component) : component(component) {}
void CEOnMembersChanged::PushArguments(lua_State *l) { component.PushLuaObject(l); }

/////////////

void CEGenericComponentEvent::PushArguments(lua_State *l) {}
