// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

#include <string>

#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.libraries.import;

export {
	class NetworkState;
	namespace Lua {
		namespace import {
			DLLNETWORK int import_wrci(lua_State *l);
			DLLNETWORK int import_wad(lua_State *l);
			DLLNETWORK int import_wrmi(lua_State *l);
			DLLNETWORK int import_smd(lua_State *l);
			DLLNETWORK int import_obj(lua_State *l);
			DLLNETWORK int import_model_asset(lua_State *l);
			DLLNETWORK bool import_model_asset(NetworkState &nw, const std::string &outputPath, std::string &outFilePath, std::string &outErr);

			DLLNETWORK int export_model_asset(lua_State *l);
		};
	};
};
