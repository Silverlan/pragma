// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.function;

LuaFunction::LuaFunction(const luabind::object &o) : m_luaFunction(new luabind::object(o)) {}
LuaFunction::LuaFunction(const LuaFunction &other) : m_luaFunction(other.m_luaFunction) {}
LuaFunction::LuaFunction(std::nullptr_t) : m_luaFunction(nullptr) {}
LuaFunction::~LuaFunction() {}
luabind::object &LuaFunction::GetLuaObject() { return *m_luaFunction.get(); }

bool LuaFunction::operator==(std::nullptr_t) { return (m_luaFunction == nullptr) ? true : false; }
void LuaFunction::operator()() { Call<void>(); }
