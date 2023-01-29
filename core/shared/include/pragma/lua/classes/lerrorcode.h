/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LERRORCODE_H__
#define __LERRORCODE_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <system_error>

#undef GetMessage

class DLLNETWORK ErrorCode {
  protected:
	std::string m_message;
	int32_t m_value;
	std::unique_ptr<std::error_code> m_error;
  public:
	ErrorCode();
	ErrorCode(const std::string &msg, int32_t value);
	ErrorCode(const std::error_code &err);
	ErrorCode(const ErrorCode &other);
	bool IsError() const;
	std::string GetMessage() const;
	int32_t GetValue() const;
};

DLLNETWORK std::ostream &operator<<(std::ostream &out, const ErrorCode &err);

//////////////////////////////

DLLNETWORK std::string Lua_ErrorCode_GetMessage(lua_State *l, ErrorCode &err);
DLLNETWORK int32_t Lua_ErrorCode_GetValue(lua_State *l, ErrorCode &err);
DLLNETWORK bool Lua_ErrorCode_IsError(lua_State *l, ErrorCode &err);

#endif
