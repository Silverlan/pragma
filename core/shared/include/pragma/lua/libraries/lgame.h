/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LGAME_H__
#define __LGAME_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

namespace Lua
{
	namespace game
	{
		DLLNETWORK int add_callback(lua_State *l);
		DLLNETWORK int call_callbacks(lua_State *l);
		DLLNETWORK int clear_callbacks(lua_State *l);
		DLLNETWORK int register_ammo_type(lua_State *l);
		DLLNETWORK int get_ammo_type_id(lua_State *l);
		DLLNETWORK int get_ammo_type_name(lua_State *l);
		DLLNETWORK int get_game_mode(lua_State *l);
		DLLNETWORK int get_light_color(lua_State *l);
		DLLNETWORK int get_sound_intensity(lua_State *l);
		DLLNETWORK int get_time_scale(lua_State *l);
		DLLNETWORK int set_time_scale(lua_State *l);
		DLLNETWORK int is_game_mode_initialized(lua_State *l);
		DLLNETWORK int raycast(lua_State *l);
		DLLNETWORK int get_nav_mesh(lua_State *l);
		DLLNETWORK int load_nav_mesh(lua_State *l);
		DLLNETWORK int is_map_loaded(lua_State *l);
		DLLNETWORK int get_map_name(lua_State *l);
		DLLNETWORK int get_game_state_flags(lua_State *l);
		DLLNETWORK std::pair<bool,int> load_map(lua_State *l,std::string &mapName,BaseEntity **entWorld,Vector3 &origin);
	};
};

#define LUA_LIB_GAME_SHARED \
	{"add_callback",Lua::game::add_callback}, \
	{"call_callbacks",Lua::game::call_callbacks}, \
	{"clear_callbacks",Lua::game::clear_callbacks}, \
	{"register_ammo_type",Lua::game::register_ammo_type}, \
	{"get_ammo_type_id",Lua::game::get_ammo_type_id}, \
	{"get_ammo_type_name",Lua::game::get_ammo_type_name}, \
	{"get_game_mode",Lua::game::get_game_mode}, \
	{"get_light_color",Lua::game::get_light_color}, \
	{"get_sound_intensity",Lua::game::get_sound_intensity}, \
	{"get_time_scale",Lua::game::get_time_scale}, \
	{"is_game_mode_initialized",Lua::game::is_game_mode_initialized}, \
	{"raycast",Lua::game::raycast}, \
	{"get_nav_mesh",Lua::game::get_nav_mesh}, \
	{"load_nav_mesh",Lua::game::load_nav_mesh}, \
	{"is_map_loaded",Lua::game::is_map_loaded}, \
	{"get_map_name",Lua::game::get_map_name}, \
	{"get_game_state_flags",Lua::game::get_game_state_flags},

#endif
