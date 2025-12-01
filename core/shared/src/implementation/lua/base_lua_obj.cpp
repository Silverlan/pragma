// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.base_lua_obj;

BaseLuaObj::BaseLuaObj() : m_luaObj(nullptr), m_bExternalHandle(false) {}

BaseLuaObj::~BaseLuaObj() { m_luaObj = nullptr; }

luabind::object *BaseLuaObj::GetLuaObject() { return m_luaObj.get(); }
