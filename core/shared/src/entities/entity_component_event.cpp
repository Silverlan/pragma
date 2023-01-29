/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_component_event.hpp"

using namespace pragma;

CEOnEntityComponentAdded::CEOnEntityComponentAdded(BaseEntityComponent &component) : component(component) {}
void CEOnEntityComponentAdded::PushArguments(lua_State *l) { component.PushLuaObject(l); }

/////////////

CEOnMembersChanged::CEOnMembersChanged(BaseEntityComponent &component) : component(component) {}
void CEOnMembersChanged::PushArguments(lua_State *l) { component.PushLuaObject(l); }

/////////////

void CEGenericComponentEvent::PushArguments(lua_State *l) {}
