/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/lskeleton.h"

#include "pragma/lua/ostream_operator_alias.hpp"
#include <panima/bone.hpp>




DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(panima,Bone);



void Lua::Bone::register_class(lua_State *l,luabind::class_<panima::Skeleton> &classDef)
{
	auto classDefBone = luabind::class_<panima::Bone>("Bone");
	classDefBone.def(luabind::tostring(luabind::self));
	classDefBone.def("GetName",&Lua::Bone::GetName);
	classDefBone.def("GetID",&Lua::Bone::GetID);
	classDefBone.def("GetChildren",&Lua::Bone::GetChildren);
	classDefBone.def("GetParent",&Lua::Bone::GetParent);
	classDefBone.def("SetName",&Lua::Bone::SetName);
	classDefBone.def("SetParent",&Lua::Bone::SetParent);
	classDefBone.def("ClearParent",&Lua::Bone::ClearParent);
	classDefBone.def("IsAncestorOf",&panima::Bone::IsAncestorOf);
	classDefBone.def("IsDescendantOf",&panima::Bone::IsDescendantOf);
	classDef.scope[classDefBone];
}
