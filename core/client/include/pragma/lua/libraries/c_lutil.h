#ifndef __C_LUTIL_H__
#define __C_LUTIL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include "pragma/lua/libraries/lutil.h"

namespace Lua
{
	namespace util
	{
		namespace Client
		{
			DLLCLIENT int calc_world_direction_from_2d_coordinates(lua_State *l);
			DLLCLIENT int create_particle_tracer(lua_State *l);
			DLLCLIENT int create_muzzle_flash(lua_State *l);
			DLLCLIENT int create_giblet(lua_State *l);
		};
	};
};

#endif
