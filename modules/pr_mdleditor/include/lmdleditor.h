#ifndef __LOPENVR_H__
#define __LOPENVR_H__

#include "mdeincludes.h"
#include <luasystem.h>

namespace Lua
{
	namespace mde
	{
		void register_lua_library(lua_State *l);
		namespace lib
		{
			int load_smd(lua_State *l);
			int load_fbx(lua_State *l);
			int load_mdl(lua_State *l);
			int load_phy(lua_State *l);
		};
	}
};

#endif