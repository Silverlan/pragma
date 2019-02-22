#ifndef __C_LGAME_H__
#define __C_LGAME_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace game
	{
		namespace Client
		{
			DLLCLIENT int open_dropped_file(lua_State *l);
			DLLCLIENT int set_gravity(lua_State *l);
			DLLCLIENT int get_gravity(lua_State *l);
			DLLCLIENT int load_model(lua_State *l);
			DLLCLIENT int create_model(lua_State *l);
			DLLCLIENT int get_action_input(lua_State *l);
			DLLCLIENT int set_action_input(lua_State *l);
			DLLCLIENT int draw_scene(lua_State *l);
			DLLCLIENT int create_scene(lua_State *l);
			DLLCLIENT int get_render_scene(lua_State *l);
			DLLCLIENT int get_render_scene_camera(lua_State *l);
			DLLCLIENT int get_scene(lua_State *l);
			DLLCLIENT int get_scene_camera(lua_State *l);
			DLLCLIENT int get_draw_command_buffer(lua_State *l);
			DLLCLIENT int get_setup_command_buffer(lua_State *l);
			DLLCLIENT int flush_setup_command_buffer(lua_State *l);
			DLLCLIENT int get_camera_position(lua_State *l);
			DLLCLIENT int get_render_clip_plane(lua_State *l);
			DLLCLIENT int set_render_clip_plane(lua_State *l);

			DLLCLIENT int get_debug_buffer(lua_State *l);
			DLLCLIENT int get_time_buffer(lua_State *l);
			DLLCLIENT int get_csm_buffer(lua_State *l);
			DLLCLIENT int get_render_settings_descriptor_set(lua_State *l);

#ifdef PHYS_ENGINE_PHYSX
			DLLCLIENT int get_physx_scene(lua_State *l);
#endif
			DLLCLIENT int load_map(lua_State *l);

			DLLCLIENT int test(lua_State *l);
		};
	};
};

#endif
