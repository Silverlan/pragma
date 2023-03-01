/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/base_lua_handle.hpp"
#include "pragma/lua/base_lua_handle_method.hpp"

pragma::BaseLuaHandle::BaseLuaHandle() : m_handle {this, [](BaseLuaHandle *) {}} {}
void pragma::BaseLuaHandle::InvalidateHandle() { m_handle = {}; }
void pragma::BaseLuaHandle::SetLuaObject(const luabind::object &o) { m_luaObj = o; }
lua_State *pragma::BaseLuaHandle::GetLuaState() const { return m_luaObj.interpreter(); }
void pragma::BaseLuaHandle::CallLuaMethod(const std::string &name) { CallLuaMethod<void>(name); }
void pragma::BaseLuaHandle::PushLuaObject()
{
	auto *l = m_luaObj.interpreter();
	if(!l)
		return;
	PushLuaObject(l);
}
void pragma::BaseLuaHandle::PushLuaObject(lua_State *l) { m_luaObj.push(l); }
pragma::BaseLuaHandle::~BaseLuaHandle() { InvalidateHandle(); }
