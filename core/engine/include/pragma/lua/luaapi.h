#ifndef __LUAAPI_H__
#define __LUAAPI_H__

#include "pragma/definitions.h"
// Has to be included before luabind!
#include "lua_handles.hpp"

#include <luasystem.h>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

namespace Lua
{
	class Interface;
	DLLENGINE void initialize_lua_state(Lua::Interface &lua);
	DLLENGINE void set_extended_lua_modules_enabled(bool b);
	DLLENGINE bool get_extended_lua_modules_enabled();
};

#endif
