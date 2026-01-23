// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.lib_import;

export import pragma.lua;

export {
	namespace pragma {
		class NetworkState;
	}
	namespace Lua {
		namespace import {
			DLLNETWORK int import_wrci(lua::State *l);
			DLLNETWORK int import_wad(lua::State *l);
			DLLNETWORK int import_wrmi(lua::State *l);
			DLLNETWORK int import_smd(lua::State *l);
			DLLNETWORK int import_obj(lua::State *l);
			DLLNETWORK int import_model_asset(lua::State *l);
			DLLNETWORK bool import_model_asset(pragma::NetworkState &nw, const std::string &outputPath, std::string &outFilePath, std::string &outErr);

			DLLNETWORK int export_model_asset(lua::State *l);
		};
	};
};
