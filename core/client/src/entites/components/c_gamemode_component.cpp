/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_gamemode_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(gamemode,CGamemode);

luabind::object CGamemodeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CGamemodeComponentHandleWrapper>(l);}

////////////

void CGamemode::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CGamemodeComponent>();
}
