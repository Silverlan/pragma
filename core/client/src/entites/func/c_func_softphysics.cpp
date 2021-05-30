/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/func/c_func_softphysics.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics_softbody,CFuncSoftPhysics);

void CFuncSoftPhysicsComponent::Initialize()
{
	BaseFuncSoftPhysicsComponent::Initialize();
	GetEntity().AddComponent("func_physics");
}
luabind::object CFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFuncSoftPhysicsComponentHandleWrapper>(l);}

///////////

void CFuncSoftPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncSoftPhysicsComponent>();
}
