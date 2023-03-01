/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lerrorcode.h"
#include "luasystem.h"

ErrorCode::ErrorCode() : m_value(0) {}
ErrorCode::ErrorCode(const std::string &msg, int32_t value) : m_message(msg), m_value(value) {}
ErrorCode::ErrorCode(const std::error_code &err) : m_value(0), m_error(new std::error_code(err)) {}

ErrorCode::ErrorCode(const ErrorCode &other)
{
	m_value = other.m_value;
	m_message = other.m_message;
	if(other.m_error != nullptr)
		m_error = std::make_unique<std::error_code>(*other.m_error);
}
bool ErrorCode::IsError() const
{
	if(m_error != nullptr)
		return *m_error ? true : false;
	return (m_value == 0) ? false : true;
}
std::string ErrorCode::GetMessage() const
{
	if(m_error != nullptr)
		return m_error->message();
	return m_message;
}
int32_t ErrorCode::GetValue() const
{
	if(m_error != nullptr)
		return m_error->value();
	return m_value;
}

std::ostream &operator<<(std::ostream &out, const ErrorCode &err)
{
	out << err.GetMessage() << " (" << err.GetValue() << ")";
	return out;
}

//////////////////////////////

std::string Lua_ErrorCode_GetMessage(lua_State *l, ErrorCode &err) { return err.GetMessage(); }
int32_t Lua_ErrorCode_GetValue(lua_State *l, ErrorCode &err) { return err.GetValue(); }
bool Lua_ErrorCode_IsError(lua_State *l, ErrorCode &err) { return err.IsError(); }
