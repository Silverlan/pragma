// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.error_code;

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

std::string Lua_ErrorCode_GetMessage(lua::State *l, ErrorCode &err) { return err.GetMessage(); }
int32_t Lua_ErrorCode_GetValue(lua::State *l, ErrorCode &err) { return err.GetValue(); }
bool Lua_ErrorCode_IsError(lua::State *l, ErrorCode &err) { return err.IsError(); }
