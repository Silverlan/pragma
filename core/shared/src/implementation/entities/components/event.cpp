// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.events.event;

using namespace pragma;

CEOnEntityComponentAdded::CEOnEntityComponentAdded(BaseEntityComponent &component) : component(component) {}
void CEOnEntityComponentAdded::PushArguments(lua::State *l) { component.PushLuaObject(l); }

/////////////

CEOnMembersChanged::CEOnMembersChanged(BaseEntityComponent &component) : component(component) {}
void CEOnMembersChanged::PushArguments(lua::State *l) { component.PushLuaObject(l); }

/////////////

void CEGenericComponentEvent::PushArguments(lua::State *l) {}
