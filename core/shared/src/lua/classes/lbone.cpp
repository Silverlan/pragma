#include "stdafx_shared.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/lskeleton.h"

void Lua::Bone::register_class(lua_State *l,luabind::class_<::Skeleton> &classDef)
{
	auto classDefBone = luabind::class_<::Bone>("Bone");
	classDefBone.def("GetName",&Lua::Bone::GetName);
	classDefBone.def("GetID",&Lua::Bone::GetID);
	classDefBone.def("GetChildren",&Lua::Bone::GetChildren);
	classDefBone.def("GetParent",&Lua::Bone::GetParent);
	classDefBone.def("SetName",&Lua::Bone::SetName);
	classDefBone.def("SetParent",&Lua::Bone::SetParent);
	classDefBone.def("ClearParent",&Lua::Bone::ClearParent);
	classDef.scope[classDefBone];
}
