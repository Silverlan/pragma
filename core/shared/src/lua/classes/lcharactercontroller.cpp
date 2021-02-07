/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/controller.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLNETWORK Engine *engine;

// TODO: Remove this file
namespace Lua
{
	namespace PhysKinematicCharacterController
	{
		static void SetOrigin(lua_State *l,pragma::physics::IController &controller,Vector3 &origin);
		static void GetOrigin(lua_State *l,::pragma::physics::IController &controller);
	};
};

void Lua::PhysKinematicCharacterController::register_class(lua_State *l,luabind::module_ &mod)
{
	/*auto classDef = luabind::class_<::pragma::physics::IController>("CharacterController");
	classDef.def("SetOrigin",&SetOrigin);
	classDef.def("GetOrigin",&GetOrigin);
	mod[classDef];*/
}
void Lua::PhysKinematicCharacterController::SetOrigin(lua_State*,::pragma::physics::IController &controller,Vector3 &origin)
{
	auto *colObj = controller.GetCollisionObject();
	colObj->SetOrigin(origin);
}

void Lua::PhysKinematicCharacterController::GetOrigin(lua_State *l,::pragma::physics::IController &controller)
{
	auto *colObj = controller.GetCollisionObject();
	Lua::Push<Vector3>(l,colObj->GetOrigin());
}
