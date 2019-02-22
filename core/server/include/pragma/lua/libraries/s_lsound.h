#ifndef __S_LSOUND_H__
#define __S_LSOUND_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <pragma/lua/libraries/lsound.h>

namespace Lua
{
	namespace sound
	{
		namespace Server
		{
			DLLSERVER int create(lua_State *l);
		};
	};
};

#endif

