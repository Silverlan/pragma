#ifndef __C_LDEF_LIGHT_HPP__
#define __C_LDEF_LIGHT_HPP__

// TODO: Remove this file
#if 0
#include <pragma/lua/ldefinitions.h>
#include "pragma/rendering/lighting/c_light.h"

#define LUA_CHECK_LIGHT(l,hLight) \
	if(!hLight.IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL light"); \
		lua_error(l); \
		return; \
	}

LUA_SETUP_HANDLE_CHECK(CGameLight,CLightBase,CLightObjectHandle);
#endif

#endif
