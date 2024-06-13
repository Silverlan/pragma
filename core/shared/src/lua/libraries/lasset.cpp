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
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include <material_manager2.hpp>
#include <luainterface.hpp>
#include <fsys/ifile.hpp>
#include <sharedutils/magic_enum.hpp>

extern DLLNETWORK Engine *engine;

static std::unordered_map<pragma::asset::Type, std::string> g_assetTypeToIdentifier;
static std::unordered_map<std::string, pragma::asset::Type> g_identifierToAssetType;
static void init_asset_type_maps()
{
	auto n = umath::to_integral(pragma::asset::Type::Count);
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		auto type = static_cast<pragma::asset::Type>(i);
		std::string strType {magic_enum::enum_name(type)};
		std::string normalizedTypeName;
		normalizedTypeName.reserve(strType.size() * 2);
		for(auto c : strType) {
			if(std::isupper(c)) {
				if(!normalizedTypeName.empty())
					normalizedTypeName += '_';
				normalizedTypeName += std::tolower(c);
			}
			else
				normalizedTypeName += c;
		}
		g_assetTypeToIdentifier[type] = normalizedTypeName;
		g_identifierToAssetType[normalizedTypeName] = type;
	}
}
static std::optional<pragma::asset::Type> get_asset_type_from_identifier(const std::string &identifier)
{
	if(g_assetTypeToIdentifier.empty())
		init_asset_type_maps();
	auto it = g_identifierToAssetType.find(identifier);
	if(it == g_identifierToAssetType.end())
		return {};
	return it->second;
}
static std::optional<std::string> get_asset_identifier_from_type(pragma::asset::Type type)
{
	if(g_assetTypeToIdentifier.empty())
		init_asset_type_maps();
	auto it = g_assetTypeToIdentifier.find(type);
	if(it == g_assetTypeToIdentifier.end())
		return {};
	return it->second;
}
void Lua::asset::register_library(Lua::Interface &lua, bool extended)
{
	auto modAsset = luabind::module_(lua.GetState(), "asset");
	modAsset[
		luabind::def("clear_unused",+[](NetworkState &nw,pragma::asset::Type type) -> std::optional<uint32_t> {
			auto *assetManager = nw.GetAssetManager(type);
			if(!assetManager)
				return {};
			return assetManager->ClearUnused();
		}),
		luabind::def("clear_unused",+[](NetworkState &nw) -> std::optional<uint32_t> {
			auto num = umath::to_integral(pragma::asset::Type::Count);
			uint32_t numCleared = 0;
			for (decltype(num) i=0u;i<num;++i)
			{
				auto type = static_cast<pragma::asset::Type>(i);
				auto *assetManager = nw.GetAssetManager(type);
				if(!assetManager)
					continue;
			    numCleared += assetManager->ClearUnused();
			}
			return numCleared;
		}),
		luabind::def("clear_unused_models",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetModelManager().ClearUnused();
		})),
		luabind::def("clear_flagged_models",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetModelManager().ClearFlagged();
		})),
		luabind::def("flag_model_for_cache_removal",static_cast<void(*)(lua_State*,Model&)>([](lua_State *l,Model &mdl) {
			auto *nw = engine->GetNetworkState(l);
			nw->GetModelManager().FlagForRemoval(mdl);
		})),
		luabind::def("clear_unused_materials",static_cast<uint32_t(*)(lua_State*)>([](lua_State *l) -> uint32_t {
			auto *nw = engine->GetNetworkState(l);
			return nw->GetMaterialManager().ClearUnused();
		})),
		luabind::def("get_type_identifier",&get_asset_identifier_from_type),
		luabind::def("get_type_enum",&get_asset_type_from_identifier),
		luabind::def("lock_asset_watchers",&::Engine::LockResourceWatchers),
		luabind::def("unlock_asset_watchers",&::Engine::UnlockResourceWatchers),
		luabind::def("poll_asset_watchers",&::Engine::PollResourceWatchers),
		luabind::def("get_supported_import_file_extensions",&Lua::asset::get_supported_import_file_extensions),
		luabind::def("get_supported_export_file_extensions",&Lua::asset::get_supported_export_file_extensions),
		luabind::def("matches",&pragma::asset::matches),
		luabind::def("get_normalized_path",&pragma::asset::get_normalized_path),
		luabind::def("is_supported_extension",+[](lua_State *l,const std::string &ext,pragma::asset::Type type) -> bool {
			auto &exts = pragma::asset::get_supported_extensions(type);
			return std::find(exts.begin(),exts.end(),ext) != exts.end();
		}),
		luabind::def("get_supported_extensions",+[](lua_State *l,pragma::asset::Type type) -> tb<std::string> {
			return Lua::vector_to_table(l,pragma::asset::get_supported_extensions(type));
		}),
		luabind::def("get_supported_extensions",+[](lua_State *l,pragma::asset::Type type,pragma::asset::FormatType formatType) -> tb<std::string> {
			return Lua::vector_to_table(l,pragma::asset::get_supported_extensions(type,formatType));
		}),
		luabind::def("get_legacy_extension",static_cast<opt<std::string>(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> opt<std::string> {
			auto ext = pragma::asset::get_legacy_extension(type);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("get_binary_udm_extension",static_cast<opt<std::string>(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> opt<std::string> {
			auto ext = pragma::asset::get_binary_udm_extension(type);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("get_ascii_udm_extension",static_cast<opt<std::string>(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> opt<std::string> {
			auto ext = pragma::asset::get_ascii_udm_extension(type);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("determine_format_from_data",static_cast<opt<std::string>(*)(lua_State*,LFile&,pragma::asset::Type)>([](lua_State *l,LFile &f,pragma::asset::Type type) -> opt<std::string> {
			auto ext = pragma::asset::determine_format_from_data(*f.GetHandle(),type);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("determine_format_from_filename",static_cast<opt<std::string>(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &fileName,pragma::asset::Type type) -> opt<std::string> {
			auto ext = pragma::asset::determine_format_from_filename(fileName,type);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("determine_type_from_extension",static_cast<opt<pragma::asset::Type>(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &ext) -> opt<pragma::asset::Type> {
			auto type = pragma::asset::determine_type_from_extension(ext);
			if(!type.has_value())
				return nil;
			return {l,*type};
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
		luabind::def("get_udm_format_extension",static_cast<opt<std::string>(*)(lua_State*,pragma::asset::Type,bool)>([](lua_State *l,pragma::asset::Type type,bool binary) -> opt<std::string> {
			auto ext = pragma::asset::get_udm_format_extension(type,binary);
			if(!ext.has_value())
				return nil;
			return {l,*ext};
		})),
		luabind::def("get_asset_root_directory",static_cast<std::string(*)(lua_State*,pragma::asset::Type)>([](lua_State *l,pragma::asset::Type type) -> std::string {
			return pragma::asset::get_asset_root_directory(type);
		})),
		luabind::def("exists",Lua::asset::exists),
		luabind::def("find_file",Lua::asset::find_file),
		luabind::def("is_loaded",Lua::asset::is_loaded),
		luabind::def("wait_until_loaded",+[](lua_State *l,const std::string &name,pragma::asset::Type type) -> bool {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return false;
			return manager->WaitUntilAssetLoaded(name);
		}),
		luabind::def("wait_until_all_pending_jobs_complete",+[](lua_State *l,const std::string &name,pragma::asset::Type type) {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return;
			manager->WaitForAllPendingCompleted();
		}),
		luabind::def("precache",+[](lua_State *l,const std::string &name,pragma::asset::Type type)
			-> Lua::var<bool,std::pair<util::FileAssetManager::PreloadResult::Result,std::optional<util::AssetLoadJobId>>> {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return luabind::object{l,false};
			auto result = manager->PreloadAsset(name);
			return luabind::object{
				l,
				std::pair<util::FileAssetManager::PreloadResult::Result,std::optional<util::AssetLoadJobId>>{
					result.result,result.jobId
				}
			};
		}),
		luabind::def("get_asset_state",+[](lua_State *l,const std::string &name,pragma::asset::Type type)
			-> Lua::opt<util::AssetState> {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return luabind::object{};
			auto result = manager->GetAssetState(name);
			return luabind::object{l,result};
		}),
		luabind::def("normalize_asset_name",+[](lua_State *l,const std::string &name,pragma::asset::Type type) -> std::string {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return name;
			return manager->ToCacheIdentifier(name);
		}),
		luabind::def("delete",static_cast<bool(*)(lua_State*,const std::string&,pragma::asset::Type)>([](lua_State *l,const std::string &name,pragma::asset::Type type) -> bool {
			return pragma::asset::remove_asset(name,type);
		})),
		luabind::def("find",+[](lua_State *l,const std::string &path,pragma::asset::Type type) -> Lua::tb<std::string> {
			auto exts = pragma::asset::get_supported_extensions(type);
			std::string rootPath = pragma::asset::get_asset_root_directory(type);
			if(!rootPath.empty())
				rootPath += '/';
			std::vector<std::string> files;
			filemanager::find_files(rootPath +path,&files,nullptr);

			std::unordered_set<std::string> extMap;
			for(auto &ext : exts)
				extMap.insert(ext);

			auto tFiles = luabind::newtable(l);
			for(uint32_t idx=1;auto &f : files)
			{
				std::string ext;
				if(ufile::get_extension(f,&ext) == false || extMap.find(ext) == extMap.end())
					continue;
				tFiles[idx++] = f;
			}
			return tFiles;
		}),
		luabind::def("poll",+[](lua_State *l,pragma::asset::Type type) {
			auto *manager = engine->GetNetworkState(l)->GetAssetManager(type);
			if(!manager)
				return;
			manager->Poll();
		}),
		luabind::def("poll_all",+[](lua_State *l) {
			auto n = umath::to_integral(pragma::asset::Type::Count);
			for(auto i=decltype(n){0u};i<n;++i)
			{
				auto *manager = engine->GetNetworkState(l)->GetAssetManager(static_cast<pragma::asset::Type>(i));
				if(!manager)
					continue;
				manager->Poll();
			}
		}),
		luabind::def("type_to_string",+[](pragma::asset::Type type) -> std::string {
		return std::string { magic_enum::enum_name(type)};
		})
	];

	Lua::RegisterLibraryEnums(lua.GetState(), "asset",
	  {{"TYPE_MODEL", umath::to_integral(pragma::asset::Type::Model)}, {"TYPE_MAP", umath::to_integral(pragma::asset::Type::Map)}, {"TYPE_MATERIAL", umath::to_integral(pragma::asset::Type::Material)}, {"TYPE_TEXTURE", umath::to_integral(pragma::asset::Type::Texture)},
	    {"TYPE_AUDIO", umath::to_integral(pragma::asset::Type::Sound)}, {"TYPE_PARTICLE_SYSTEM", umath::to_integral(pragma::asset::Type::ParticleSystem)},

	    {"FORMAT_TYPE_NATIVE", umath::to_integral(pragma::asset::FormatType::Native)}, {"FORMAT_TYPE_IMPORT", umath::to_integral(pragma::asset::FormatType::Import)}, {"FORMAT_TYPE_ALL", umath::to_integral(pragma::asset::FormatType::All)},

	    {"ASSET_LOAD_FLAG_NONE", umath::to_integral(util::AssetLoadFlags::None)}, {"ASSET_LOAD_FLAG_ABSOLUTE_PATH_BIT", umath::to_integral(util::AssetLoadFlags::AbsolutePath)}, {"ASSET_LOAD_FLAG_DONT_CACHE_BIT", umath::to_integral(util::AssetLoadFlags::DontCache)},
	    {"ASSET_LOAD_FLAG_IGNORE_CACHE_BIT", umath::to_integral(util::AssetLoadFlags::IgnoreCache)},

	    {"ASSET_STATE_NOT_LOADED", umath::to_integral(util::AssetState::NotLoaded)}, {"ASSET_STATE_LOADED", umath::to_integral(util::AssetState::Loaded)}, {"ASSET_STATE_FAILED_TO_LOAD", umath::to_integral(util::AssetState::FailedToLoad)},
	    {"ASSET_STATE_LOADING", umath::to_integral(util::AssetState::Loading)}});
	static_assert(umath::to_integral(pragma::asset::Type::Count) == 6, "Update this list!");

	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MAP_BINARY", pragma::asset::FORMAT_MAP_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MAP_ASCII", pragma::asset::FORMAT_MAP_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MAP_LEGACY", pragma::asset::FORMAT_MAP_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MODEL_BINARY", pragma::asset::FORMAT_MODEL_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MODEL_ASCII", pragma::asset::FORMAT_MODEL_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MODEL_LEGACY", pragma::asset::FORMAT_MODEL_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_PARTICLE_SYSTEM_BINARY", pragma::asset::FORMAT_PARTICLE_SYSTEM_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_PARTICLE_SYSTEM_ASCII", pragma::asset::FORMAT_PARTICLE_SYSTEM_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_PARTICLE_SYSTEM_LEGACY", pragma::asset::FORMAT_PARTICLE_SYSTEM_LEGACY);

	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MATERIAL_BINARY", pragma::asset::FORMAT_MATERIAL_BINARY);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MATERIAL_ASCII", pragma::asset::FORMAT_MATERIAL_ASCII);
	Lua::RegisterLibraryValue<std::string>(lua.GetState(), "asset", "FORMAT_MATERIAL_LEGACY", pragma::asset::FORMAT_MATERIAL_LEGACY);
}
bool Lua::asset::exists(lua_State *l, const std::string &name, pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	return pragma::asset::exists(name, type);
}
Lua::opt<std::string> Lua::asset::find_file(lua_State *l, const std::string &name, pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	auto path = pragma::asset::find_file(name, type);
	if(path.has_value() == false)
		return nil;
	return {l, *path};
}
bool Lua::asset::is_loaded(lua_State *l, const std::string &name, pragma::asset::Type type)
{
	auto *nw = engine->GetNetworkState(l);
	return pragma::asset::is_loaded(*nw, name, type);
}
Lua::tb<std::string> Lua::asset::get_supported_import_file_extensions(lua_State *l, pragma::asset::Type type)
{
	auto t = luabind::newtable(l);
	auto &assetManager = engine->GetAssetManager();
	auto n = assetManager.GetImporterCount(type);
	int32_t idx = 1;
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		for(auto &ext : assetManager.GetImporterInfo(type, i)->fileExtensions)
			t[idx++] = ext.first;
	}
	if(type == pragma::asset::Type::Model) {
		// These are implemented using the old importer system, so they're not included in the import information
		// retrieved above. We'll add them to the list manually for now.
		// TODO: Move these to the new importer system and remove these entries!
		t[idx++] = "mdl";
		t[idx++] = "vmdl_c";
		t[idx++] = "nif";
	}
	else if(type == pragma::asset::Type::Material) {
		// These are implemented using the old importer system, so they're not included in the import information
		// retrieved above. We'll add them to the list manually for now.
		// TODO: Move these to the new importer system and remove these entries!
		t[idx++] = "vmt";
		t[idx++] = "vmat_c";
	}
	else if(type == pragma::asset::Type::Map) {
		// These are implemented using the old importer system, so they're not included in the import information
		// retrieved above. We'll add them to the list manually for now.
		// TODO: Move these to the new importer system and remove these entries!
		t[idx++] = "bsp";
	}
	else if(type == pragma::asset::Type::ParticleSystem)
		t[idx++] = "pcf";
	return t;
}
Lua::tb<std::string> Lua::asset::get_supported_export_file_extensions(lua_State *l, pragma::asset::Type type)
{
	auto t = luabind::newtable(l);
	auto &assetManager = engine->GetAssetManager();
	auto n = assetManager.GetExporterCount(type);
	int32_t idx = 1;
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		for(auto &ext : assetManager.GetExporterInfo(type, i)->fileExtensions)
			t[idx++] = ext.first;
	}
	return t;
}
