#include "stdafx_shared.h"
#include "pragma/lua/classes/lmaterial.h"
#include "luasystem.h"
#include "material.h"
#include <sharedutils/util_shaderinfo.hpp>

void Lua::Material::register_class(luabind::class_<::Material> &classDef)
{
	classDef.def("IsValid",&Lua::Material::IsValid);
	classDef.def("GetShaderName",&Lua::Material::GetShaderName);
	classDef.def("GetName",&Lua::Material::GetName);
	classDef.def("IsTranslucent",&Lua::Material::IsTranslucent);
	classDef.def("GetDataBlock",&Lua::Material::GetDataBlock);
}

void Lua::Material::IsTranslucent(lua_State *l,::Material &mat)
{
	Lua::PushBool(l,mat.IsTranslucent());
}

void Lua::Material::IsValid(lua_State *l,::Material &mat)
{
	Lua::PushBool(l,mat.IsValid());
}

void Lua::Material::GetName(lua_State *l,::Material &mat)
{
	Lua::PushString(l,mat.GetName());
}

void Lua::Material::GetShaderName(lua_State *l,::Material &mat) {Lua::PushString(l,mat.GetShaderIdentifier());}

void Lua::Material::GetDataBlock(lua_State *l,::Material &mat)
{
	auto &dataBlock = mat.GetDataBlock();
	Lua::Push<std::shared_ptr<ds::Block>>(l,dataBlock);
}
