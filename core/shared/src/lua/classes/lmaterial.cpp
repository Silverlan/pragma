#include "stdafx_shared.h"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/lua/libraries/lfile.h"
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
	classDef.def("Copy",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		auto *matCopy = mat.Copy();
		if(matCopy == nullptr)
			return;
		Lua::Push<::Material*>(l,matCopy);
	}));
	classDef.def("UpdateTextures",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		mat.UpdateTextures();
	}));
	classDef.def("Save",static_cast<void(*)(lua_State*,::Material&,const std::string&)>([](lua_State *l,::Material &mat,const std::string &fileName) {
		auto matFileName = fileName;
		std::string rootPath;
		if(Lua::file::validate_write_operation(l,matFileName,rootPath) == false)
		{
			Lua::PushBool(l,false);
			return;
		}
		Lua::PushBool(l,mat.Save(matFileName,rootPath));
	}));
	classDef.def("IsError",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		Lua::PushBool(l,mat.IsError());
	}));
	classDef.def("IsLoaded",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		Lua::PushBool(l,mat.IsLoaded());
	}));
	classDef.def("IsTranslucent",static_cast<void(*)(lua_State*,::Material&)>([](lua_State *l,::Material &mat) {
		Lua::PushBool(l,mat.IsTranslucent());
	}));
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

