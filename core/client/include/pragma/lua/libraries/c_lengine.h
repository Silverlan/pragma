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
		DLLCLIENT int create_material(lua_State *l);
		DLLCLIENT int create_particle_system(lua_State *l);
		DLLCLIENT int precache_particle_system(lua_State *l);
		DLLCLIENT int save_particle_system(lua_State *l);
		DLLCLIENT int create_font(lua_State *l);
		DLLCLIENT int get_font(lua_State *l);
		DLLCLIENT int get_text_size(lua_State *l);
		DLLCLIENT int save_frame_buffer_as_tga(lua_State *l);
		DLLCLIENT int save_texture_as_tga(lua_State *l);
		DLLCLIENT int create_texture(lua_State *l);
	};
};

#endif
