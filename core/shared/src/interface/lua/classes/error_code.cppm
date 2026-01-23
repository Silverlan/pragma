// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.error_code;

export import pragma.lua;

#undef GetMessage

export {
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

	DLLNETWORK std::string Lua_ErrorCode_GetMessage(lua::State *l, ErrorCode &err);
	DLLNETWORK int32_t Lua_ErrorCode_GetValue(lua::State *l, ErrorCode &err);
	DLLNETWORK bool Lua_ErrorCode_IsError(lua::State *l, ErrorCode &err);
};
