#ifndef __S_LUTIL_H__
#define __S_LUTIL_H__

#include "pragma/serverdefinitions.h"
#include <luasystem.h>

namespace Lua
{
	namespace util
	{
		namespace Server
		{
			DLLSERVER int fire_bullets(lua_State *l);
			DLLSERVER int create_explosion(lua_State *l);
			DLLSERVER int create_giblet(lua_State *l);
		};
	};
};

#endif
