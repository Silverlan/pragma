#ifndef __S_LGAME_H__
#define __S_LGAME_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#ifdef PHYS_ENGINE_PHYSX
DLLSERVER int Lua_sv_game_GetPhysXScene(lua_State *l);
#endif

namespace Lua
{
	namespace game
	{
		namespace Server
		{
			DLLSERVER int set_gravity(lua_State *l);
			DLLSERVER int get_gravity(lua_State *l);
			DLLSERVER int load_model(lua_State *l);
			DLLSERVER int create_model(lua_State *l);
			DLLSERVER int load_map(lua_State *l);
			DLLSERVER int change_level(lua_State *l);
		};
	};
};

#endif
