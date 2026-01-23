// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.base_lua_handle;

pragma::BaseLuaHandle::BaseLuaHandle() : m_handle {this, [](BaseLuaHandle *) {}} {}
void pragma::BaseLuaHandle::InvalidateHandle() { m_handle = {}; }
void pragma::BaseLuaHandle::SetLuaObject(const luabind::object &o) { m_luaObj = o; }
lua::State *pragma::BaseLuaHandle::GetLuaState() const { return m_luaObj.interpreter(); }
void pragma::BaseLuaHandle::CallLuaMethod(const std::string &name) { CallLuaMethod<void>(name); }
void pragma::BaseLuaHandle::PushLuaObject()
{
	auto *l = m_luaObj.interpreter();
	if(!l)
		return;
	PushLuaObject(l);
}
void pragma::BaseLuaHandle::PushLuaObject(lua::State *l) { m_luaObj.push(l); }
pragma::BaseLuaHandle::~BaseLuaHandle() { InvalidateHandle(); }
