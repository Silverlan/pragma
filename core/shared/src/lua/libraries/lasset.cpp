/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lasset.hpp"
#include "pragma/model/modelmanager.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <luainterface.hpp>

extern DLLNETWORK Engine *engine;


void Lua::asset::register_library(Lua::Interface &lua,bool extended)
{
	auto modAsset = luabind::module_(lua.GetState(),"asset");
	modAsset[
		luabind::def("clear_unused_models",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetModelManager().ClearUnused();
		})),
		luabind::def("clear_flagged_models",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetModelManager().ClearFlagged();
		})),
		luabind::def("flag_model_for_cache_removal",static_cast<void(*)(lua_State*)>([](lua_State *l) {
			auto *nw = engine->GetNetworkState(l);
			auto &mdl = Lua::Check<Model>(l,1);
			nw->GetModelManager().FlagForRemoval(mdl);
		})),
		luabind::def("clear_unused_materials",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetMaterialManager().ClearUnused();
		})),
		luabind::def("lock_asset_watchers",&Lua::asset::lock_asset_watchers),
		luabind::def("unlock_asset_watchers",&Lua::asset::unlock_asset_watchers),
		luabind::def("get_supported_import_file_extensions",&Lua::asset::get_supported_import_file_extensions),
		luabind::def("get_supported_export_file_extensions",&Lua::asset::get_supported_export_file_extensions),
		luabind::def("matches",&pragma::asset::matches),
		luabind::def("get_normalized_path",&pragma::asset::get_normalized_path),
		luabind::def("get_supported_extensions",static_cast<luabind::object(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> luabind::object {
			return Lua::vector_to_table(l,pragma::asset::get_supported_extensions(type));
		})),
		luabind::def("get_legacy_extension",static_cast<luabind::object(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> luabind::object {
			auto ext = pragma::asset::get_legacy_extension(type);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("get_binary_udm_extension",static_cast<luabind::object(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> luabind::object {
			auto ext = pragma::asset::get_binary_udm_extension(type);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("get_ascii_udm_extension",static_cast<luabind::object(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> luabind::object {
			auto ext = pragma::asset::get_ascii_udm_extension(type);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("determine_format_from_data",static_cast<luabind::object(*)(lua_State*,LFile&,pragma::asset::Type)>([](lua_State *l,LFile &f,pragma::asset::Type type) -> luabind::object {
			auto ext = pragma::asset::determine_format_from_data(f.GetHandle(),type);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("determine_format_from_filename",static_cast<luabind::object(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &fileName,pragma::asset::Type type) -> luabind::object {
			auto ext = pragma::asset::determine_format_from_filename(fileName,type);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("determine_type_from_extension",static_cast<luabind::object(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &ext) -> luabind::object {
			auto type = pragma::asset::determine_type_from_extension(ext);
			if(!type.has_value())
				return {};
			return luabind::object{l,umath::to_integral(*type)};
		})),
		luabind::def("matches_format",static_cast<bool(*)(lua_State*,const std::string&,const std::string&)>([](lua_State *l,const std::string &format0,const std::string &format1) -> bool {
			return pragma::asset::matches_format(format0,format1);
		})),
		luabind::def("relative_path_to_absolute_path",static_cast<std::string(*)(lua_State*,const std::string&,pragma::asset::Type,const std::string&)>([](lua_State *l,const std::string &path,pragma::asset::Type type,const std::string &rootPath) -> std::string {
			return pragma::asset::relative_path_to_absolute_path(path,type,rootPath).GetString();
		})),
		luabind::def("relative_path_to_absolute_path",static_cast<std::string(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &path,pragma::asset::Type type) -> std::string {
			return pragma::asset::relative_path_to_absolute_path(path,type).GetString();
		})),
		luabind::def("absolute_path_to_relative_path",static_cast<std::string(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &path,pragma::asset::Type type) -> std::string {
			return pragma::asset::absolute_path_to_relative_path(path,type).GetString();
		})),
		luabind::def("get_udm_format_extension",static_cast<luabind::object(*)(lua_State*,pragma::asset::Type,bool)>([](lua_State *l,pragma::asset::Type type,bool binary) -> luabind::object {
			auto ext = pragma::asset::get_udm_format_extension(type,binary);
			if(!ext.has_value())
				return {};
			return luabind::object{l,*ext};
		})),
		luabind::def("get_asset_root_directory",static_cast<std::string(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> std::string {
			return pragma::asset::get_asset_root_directory(type);
		})),
		luabind::def("exists",Lua::asset::exists),
		luabind::def("find_file",Lua::asset::find_file),
		luabind::def("is_loaded",Lua::asset::is_loaded),
		luabind::def("delete",static_cast<bool(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &name,pragma::asset::Type type) -> bool {
			return pragma::asset::remove_asset(name,type);
		}))
	];

	Lua::RegisterLibraryEnums(lua.GetState(),"asset",{
		{"TYPE_MODEL",umath::to_integral(pragma::asset::Type::Model)},
		{"TYPE_MAP",umath::to_integral(pragma::asset::Type::Map)},
		{"TYPE_MATERIAL",umath::to_integral(pragma::asset::Type::Material)},
		{"TYPE_TEXTURE",umath::to_integral(pragma::asset::Type::Texture)},
		{"TYPE_AUDIO",umath::to_integral(pragma::asset::Type::Sound)},
		{"TYPE_PARTICLE_SYSTEM",umath::to_integral(pragma::asset::Type::ParticleSystem)}
	});
	static_assert(umath::to_integral(pragma::asset::Type::Count) == 6,"Update this list!");

	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MAP_BINARY",pragma::asset::FORMAT_MAP_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MAP_ASCII",pragma::asset::FORMAT_MAP_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MAP_LEGACY",pragma::asset::FORMAT_MAP_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MODEL_BINARY",pragma::asset::FORMAT_MODEL_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MODEL_ASCII",pragma::asset::FORMAT_MODEL_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MODEL_LEGACY",pragma::asset::FORMAT_MODEL_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_PARTICLE_SYSTEM_BINARY",pragma::asset::FORMAT_PARTICLE_SYSTEM_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_PARTICLE_SYSTEM_ASCII",pragma::asset::FORMAT_PARTICLE_SYSTEM_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_PARTICLE_SYSTEM_LEGACY",pragma::asset::FORMAT_PARTICLE_SYSTEM_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MATERIAL_BINARY",pragma::asset::FORMAT_MATERIAL_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MATERIAL_ASCII",pragma::asset::FORMAT_MATERIAL_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","FORMAT_MATERIAL_LEGACY",pragma::asset::FORMAT_MATERIAL_LEGACY);
}
bool Lua::asset::exists(lua_State *l,const std::string &name,pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	return pragma::asset::exists(name,type);
}
luabind::object Lua::asset::find_file(lua_State *l,const std::string &name,pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	auto path = pragma::asset::find_file(name,type);
	if(path.has_value() == false)
		return {};
	return luabind::object{l,*path};
}
bool Lua::asset::is_loaded(lua_State *l,const std::string &name,pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	return pragma::asset::is_loaded(*nw,name,type);
}
void Lua::asset::lock_asset_watchers(lua_State *l)
{
	engine->LockResourceWatchers();
}
void Lua::asset::unlock_asset_watchers(lua_State *l)
{
	engine->UnlockResourceWatchers();
}
luabind::object Lua::asset::get_supported_import_file_extensions(lua_State *l,pragma::asset::Type type)
{
	auto t = luabind::newtable(l);
	auto &assetManager = engine->GetAssetManager();
	auto n = assetManager.GetImporterCount(type);
	int32_t idx = 1;
	for(auto i=decltype(n){0u};i<n;++i)
	{
		for(auto &ext : assetManager.GetImporterInfo(type,i)->fileExtensions)
			t[idx++] = ext;
	}
	if(type == pragma::asset::Type::Model)
	{
		// These are implemented using the old importer system, so they're not included in the import information
		// retrieved above. We'll add them to the list manually for now.
		// TODO: Move these to the new importer system and remove these entries!
		t[idx++] = "mdl";
		t[idx++] = "vmdl_c";
		t[idx++] = "nif";
	}
	else if(type == pragma::asset::Type::Material)
	{
		// These are implemented using the old importer system, so they're not included in the import information
		// retrieved above. We'll add them to the list manually for now.
		// TODO: Move these to the new importer system and remove these entries!
		t[idx++] = "vmt";
		t[idx++] = "vmat_c";
	}
	return t;
}
luabind::object Lua::asset::get_supported_export_file_extensions(lua_State *l,pragma::asset::Type type)
{
	auto t = luabind::newtable(l);
	auto &assetManager = engine->GetAssetManager();
	auto n = assetManager.GetExporterCount(type);
	int32_t idx = 1;
	for(auto i=decltype(n){0u};i<n;++i)
	{
		for(auto &ext : assetManager.GetExporterInfo(type,i)->fileExtensions)
			t[idx++] = ext;
	}
	return t;
}
