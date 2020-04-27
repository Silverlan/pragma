/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lasset.hpp"
#include "pragma/model/modelmanager.h"
#include "pragma/asset/util_asset.hpp"
#include <luainterface.hpp>

extern DLLENGINE Engine *engine;


void Lua::asset::register_library(Lua::Interface &lua,bool extended)
{
	std::unordered_map<std::string,int32_t(*)(lua_State*)> functions = {
		{"clear_unused_models",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *nw = engine->GetNetworkState(l);
			Lua::PushInt(l,nw->GetModelManager().ClearUnused());
			return 1;
		})},
		{"clear_flagged_models",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *nw = engine->GetNetworkState(l);
			Lua::PushInt(l,nw->GetModelManager().ClearFlagged());
			return 1;
		})},
		{"flag_model_for_cache_removal",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *nw = engine->GetNetworkState(l);
			auto &mdl = Lua::Check<Model>(l,1);
			nw->GetModelManager().FlagForRemoval(mdl);
			return 0;
		})},
		{"clear_unused_materials",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *nw = engine->GetNetworkState(l);
			Lua::PushInt(l,nw->GetMaterialManager().ClearUnused());
			return 1;
		})}
	};
	if(extended)
	{
		functions.insert(std::make_pair("clear_unused_models",Lua::asset::exists));
		functions.insert(std::make_pair("clear_unused_models",Lua::asset::find_file));
		functions.insert(std::make_pair("is_loaded",Lua::asset::is_loaded));
	}
	lua.RegisterLibrary("asset",functions);

	Lua::RegisterLibraryEnums(lua.GetState(),"asset",{
		{"TYPE_MODEL",umath::to_integral(pragma::asset::Type::Model)},
		{"TYPE_MATERIAL",umath::to_integral(pragma::asset::Type::Material)},
		{"TYPE_TEXTURE",umath::to_integral(pragma::asset::Type::Texture)},
		{"TYPE_AUDIO",umath::to_integral(pragma::asset::Type::Sound)}
	});
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","MODEL_FILE_EXTENSION","wmd");
	Lua::RegisterLibraryValue<std::string>(lua.GetState(),"asset","MATERIAL_FILE_EXTENSION","wmi");
}
int32_t Lua::asset::exists(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l,2));
	auto *nw = engine->GetNetworkState(l);
	Lua::PushBool(l,pragma::asset::exists(*nw,name,type));
	return 1;
}
int32_t Lua::asset::find_file(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l,2));
	auto *nw = engine->GetNetworkState(l);
	auto path = pragma::asset::find_file(*nw,name,type);
	if(path.has_value() == false)
		return 0;
	Lua::PushString(l,*path);
	return 1;
}
int32_t Lua::asset::is_loaded(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l,2));
	auto *nw = engine->GetNetworkState(l);
	Lua::PushBool(l,pragma::asset::is_loaded(*nw,name,type));
	return 1;
}

