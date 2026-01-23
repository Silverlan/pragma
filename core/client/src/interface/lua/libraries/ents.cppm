// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.ents;

export import pragma.lua;

export namespace Lua {
	namespace ents {
		namespace Client {
			DLLCLIENT int get_local_player(lua::State *l);
			DLLCLIENT int get_listener(lua::State *l);
			DLLCLIENT int get_view_model(lua::State *l);
			DLLCLIENT int get_view_body(lua::State *l);
			DLLCLIENT int get_instance_buffer(lua::State *l);
			DLLCLIENT int get_instance_bone_buffer(lua::State *l);
		};
	};
};
