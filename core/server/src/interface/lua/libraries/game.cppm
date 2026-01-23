// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.game;

export import pragma.lua;
export import pragma.math;

export namespace Lua {
	namespace game {
		namespace Server {
			DLLSERVER void set_gravity(const Vector3 &gravity);
			DLLSERVER Vector3 get_gravity();
			DLLSERVER luabind::object load_model(lua::State *l, const std::string &name);
			DLLSERVER int create_model(lua::State *l);
			DLLSERVER int load_map(lua::State *l);
			DLLSERVER void change_level(const std::string &mapName, const std::string &landmarkName);
			DLLSERVER void change_level(const std::string &mapName);
		};
	};
};
