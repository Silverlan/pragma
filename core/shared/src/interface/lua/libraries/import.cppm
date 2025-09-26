// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

export module pragma.client:scripting.lua.libraries.import;

export namespace Lua {
	namespace import {
		DLLNETWORK int import_wrci(lua_State *l);
		DLLNETWORK int import_wad(lua_State *l);
		DLLNETWORK int import_wrmi(lua_State *l);
		DLLNETWORK int import_smd(lua_State *l);
		DLLNETWORK int import_obj(lua_State *l);
		DLLNETWORK int import_model_asset(lua_State *l);
		DLLNETWORK bool import_model_asset(NetworkState &nw, const std::string &outputPath, std::string &outFilePath, std::string &outErr);

		DLLNETWORK std::shared_ptr<aiScene> snapshot_to_assimp_scene(const pragma::SceneSnapshot &snapshot);
		DLLNETWORK int export_model_asset(lua_State *l);
	};
};
