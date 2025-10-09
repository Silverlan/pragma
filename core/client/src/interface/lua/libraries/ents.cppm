// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:scripting.lua.libraries.ents;
export namespace Lua {
	namespace ents {
		namespace Client {
			DLLCLIENT int get_local_player(lua_State *l);
			DLLCLIENT int get_listener(lua_State *l);
			DLLCLIENT int get_view_model(lua_State *l);
			DLLCLIENT int get_view_body(lua_State *l);
			DLLCLIENT int get_instance_buffer(lua_State *l);
			DLLCLIENT int get_instance_bone_buffer(lua_State *l);
		};
	};
};
