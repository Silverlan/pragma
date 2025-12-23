// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.model;

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::animation, Bone);
#endif

void Lua::Bone::register_class(lua::State *l, luabind::class_<pragma::animation::Skeleton> &classDef)
{
	auto classDefBone = luabind::class_<pragma::animation::Bone>("Bone");
	classDefBone.def(luabind::tostring(luabind::self));
	classDefBone.def("GetName", &GetName);
	classDefBone.def("GetID", &GetID);
	classDefBone.def("GetChildren", &GetChildren);
	classDefBone.def("GetParent", &GetParent);
	classDefBone.def("SetName", &SetName);
	classDefBone.def("SetParent", &SetParent);
	classDefBone.def("ClearParent", &ClearParent);
	classDefBone.def("IsAncestorOf", &pragma::animation::Bone::IsAncestorOf);
	classDefBone.def("IsDescendantOf", &pragma::animation::Bone::IsDescendantOf);
	classDef.scope[classDefBone];
}
