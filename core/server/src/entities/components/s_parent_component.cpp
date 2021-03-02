/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_parent_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SParentComponent::Initialize()
{
	BaseParentComponent::Initialize();
}
luabind::object SParentComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SParentComponentHandleWrapper>(l);}
