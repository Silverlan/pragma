/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"

using namespace pragma;

MapComponent::MapComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void MapComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("name");
}

luabind::object MapComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<MapComponentHandleWrapper>(l);}

void MapComponent::SetMapIndex(unsigned int idx) {m_mapIndex = idx;}
unsigned int MapComponent::GetMapIndex() const {return m_mapIndex;}
