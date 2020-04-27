/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/point/s_point_target.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_target,PointTarget);

luabind::object SPointTargetComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPointTargetComponentHandleWrapper>(l);}

void PointTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointTargetComponent>();
}
