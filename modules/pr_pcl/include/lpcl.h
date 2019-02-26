#ifndef __LOPENVR_H__
#define __LOPENVR_H__

#include <luasystem.h>

namespace Lua
{
	namespace pcl
	{
		void register_lua_library(lua_State *lua);
		namespace lib
		{
			int generate_poly_mesh(lua_State *l);
		};
	}
};

#endif