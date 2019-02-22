#ifndef __LUA_ERROR_HANDLING_HPP__
#define __LUA_ERROR_HANDLING_HPP__

#include "pragma/definitions.h"

namespace Lua
{
	DLLENGINE void initialize_error_handler();
	DLLENGINE int HandleTracebackError(lua_State *l);
	// Note: This function will attempt to retrieve the file name from the error message.
	// If the file name is truncated, this will not work! To be sure, define the third parameter as the actual file name.
	DLLENGINE void HandleSyntaxError(lua_State *l,Lua::StatusCode r);
	DLLENGINE void HandleSyntaxError(lua_State *l,Lua::StatusCode r,const std::string &fileName);
};

#endif
