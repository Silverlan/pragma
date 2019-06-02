#ifndef __LENGINE_H__
#define __LENGINE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

#undef LoadLibrary

namespace Lua
{
	namespace engine
	{
		DLLNETWORK int CreateLight(lua_State *l);
		DLLNETWORK int RemoveLights(lua_State *l);
		DLLNETWORK int CreateSprite(lua_State *l);
		DLLNETWORK int PrecacheModel_sv(lua_State *l);
		DLLNETWORK int LoadSoundScripts(lua_State *l);
		DLLNETWORK int LoadLibrary(lua_State *l);
		DLLNETWORK int GetTickCount(lua_State *l);

		DLLNETWORK int32_t set_record_console_output(lua_State *l);
		DLLNETWORK int32_t poll_console_output(lua_State *l);
	};
};

#endif
