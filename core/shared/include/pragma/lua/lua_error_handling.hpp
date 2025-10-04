// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_ERROR_HANDLING_HPP__
#define __LUA_ERROR_HANDLING_HPP__

#include "pragma/networkdefinitions.h"
#include <sstream>
#include "pragma/lua/luaapi.h"

namespace Lua {
	DLLNETWORK void initialize_error_handler();
	DLLNETWORK bool get_callstack(lua_State *l, std::stringstream &ss);
	DLLNETWORK int HandleTracebackError(lua_State *l);
	DLLNETWORK bool PrintTraceback(lua_State *l, std::stringstream &ssOut, const std::string *pOptErrMsg = nullptr, std::string *optOutFormattedErrMsg = nullptr);
	DLLNETWORK void PrintTraceback(lua_State *l, const std::string *pOptErrMsg = nullptr);
	// Note: This function will attempt to retrieve the file name from the error message.
	// If the file name is truncated, this will not work! To be sure, define the third parameter as the actual file name.
	DLLNETWORK void HandleSyntaxError(lua_State *l, Lua::StatusCode r);
	DLLNETWORK void HandleSyntaxError(lua_State *l, Lua::StatusCode r, const std::string &fileName);

	DLLNETWORK void OpenFileInZeroBrane(const std::string &fname, uint32_t lineId);
	DLLNETWORK std::optional<std::string> GetLuaFilePath(const std::string &fname);
};

#endif
