// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:scripting.lua.libraries.import_lib;
export namespace Lua {
	namespace lib_export {
		DLLCLIENT int export_scene(lua_State *l);
	};
};
