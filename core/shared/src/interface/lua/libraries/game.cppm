// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string>

#include <cinttypes>
#include <memory>

export module pragma.shared:scripting.lua.libraries.game;

export import :game;

export {
	namespace Lua {
		namespace game {
			// Standard Lua API
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4);
			DLLNETWORK luabind::object call_callbacks(lua_State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4, luabind::object arg5);

			DLLNETWORK opt<type<CallbackHandle>> add_callback(lua_State *l, const std::string &identifier, const func<void> &function);
			DLLNETWORK void clear_callbacks(lua_State *l, const std::string &identifier);
			DLLNETWORK bool register_ammo_type(lua_State *l, const std::string &name, int32_t damage = 10, float force = 200.f, DAMAGETYPE damageType = DAMAGETYPE::BULLET);
			DLLNETWORK opt<uint32_t> get_ammo_type_id(lua_State *l, const std::string &name);
			DLLNETWORK opt<std::string> get_ammo_type_name(lua_State *l, uint32_t typeId);
			DLLNETWORK opt<type<pragma::BaseGamemodeComponent>> get_game_mode(lua_State *l);
			DLLNETWORK opt<Vector3> get_light_color(lua_State *l, const Vector3 &pos);
			DLLNETWORK float get_sound_intensity(lua_State *l, const Vector3 &pos);
			DLLNETWORK float get_time_scale(lua_State *l);
			DLLNETWORK void set_time_scale(lua_State *l, float timeScale);
			DLLNETWORK bool is_game_mode_initialized(lua_State *l);
			DLLNETWORK bool raycast(lua_State *l, const ::TraceData &data);
			;
			DLLNETWORK opt<std::shared_ptr<pragma::nav::Mesh>> get_nav_mesh(lua_State *l);
			DLLNETWORK bool load_nav_mesh(lua_State *l, bool reload = false);
			DLLNETWORK bool is_map_loaded(lua_State *l);
			DLLNETWORK std::string get_map_name(lua_State *l);
			DLLNETWORK pragma::Game::GameFlags get_game_state_flags(lua_State *l);
			DLLNETWORK std::pair<bool, int> load_map(lua_State *l, std::string &mapName, pragma::ecs::BaseEntity **entWorld, Vector3 &origin);

			DLLNETWORK void register_shared_functions(lua_State *l, luabind::module_ &modGame);
		};
	};
};
