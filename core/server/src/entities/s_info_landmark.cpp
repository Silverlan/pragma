/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/info/s_info_landmark.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(info_landmark,InfoLandmark);

luabind::object SInfoLandmarkComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SInfoLandmarkComponentHandleWrapper>(l);}

void InfoLandmark::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SInfoLandmarkComponent>();
}
