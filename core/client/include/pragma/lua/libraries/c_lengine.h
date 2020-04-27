/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LENGINE_H__
#define __C_LENGINE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace engine
	{
		DLLCLIENT int bind_key(lua_State *l);
		DLLCLIENT int unbind_key(lua_State *l);
		DLLCLIENT int precache_material(lua_State *l);
		DLLCLIENT int precache_model(lua_State *l);
		DLLCLIENT int load_material(lua_State *l);
		DLLCLIENT int load_texture(lua_State *l);
		DLLCLIENT int get_error_material(lua_State *l);
		DLLCLIENT int clear_unused_materials(lua_State *l);
		DLLCLIENT int create_material(lua_State *l);
		DLLCLIENT int create_particle_system(lua_State *l);
		DLLCLIENT int precache_particle_system(lua_State *l);
		DLLCLIENT int save_particle_system(lua_State *l);
		DLLCLIENT int create_font(lua_State *l);
		DLLCLIENT int get_font(lua_State *l);
		DLLCLIENT int get_text_size(lua_State *l);
		DLLCLIENT int create_texture(lua_State *l);
		DLLCLIENT int get_staging_render_target(lua_State *l);
		DLLCLIENT int set_fixed_frame_delta_time_interpretation(lua_State *l);
		DLLCLIENT int clear_fixed_frame_delta_time_interpretation(lua_State *l);
		DLLCLIENT int set_tick_delta_time_tied_to_frame_rate(lua_State *l);
		DLLCLIENT int get_window_resolution(lua_State *l);
		DLLCLIENT int get_render_resolution(lua_State *l);
	};
};

#define LUA_SHARED_CL_ENGINE_FUNCTIONS \
	{"create_font",&Lua::engine::create_font}, \
	{"get_font",&Lua::engine::get_font}, \
	{"set_record_console_output",&Lua::engine::set_record_console_output}, \
	{"poll_console_output",&Lua::engine::poll_console_output}, \
	{"get_staging_render_target",&Lua::engine::get_staging_render_target}, \
	{"set_fixed_frame_delta_time_interpretation",&Lua::engine::set_fixed_frame_delta_time_interpretation}, \
	{"clear_fixed_frame_delta_time_interpretation",&Lua::engine::clear_fixed_frame_delta_time_interpretation}, \
	{"set_tick_delta_time_tied_to_frame_rate",&Lua::engine::set_tick_delta_time_tied_to_frame_rate}, \
	{"get_window_resolution",&Lua::engine::get_window_resolution}, \
	{"get_render_resolution",&Lua::engine::get_render_resolution},

#endif
