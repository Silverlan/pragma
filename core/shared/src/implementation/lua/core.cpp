// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.core;

void Lua::TypeError(const luabind::object &o, Type type)
{
	// TODO
	auto *l = o.interpreter();
	o.push(l);
	auto *typeName = Lua::GetTypeString(l, -1);
	Lua::Pop(l, 1);
	Lua::PushString(l, typeName);
	Lua::Error(o.interpreter());
}
Lua::Type Lua::GetType(const luabind::object &o) { return static_cast<Type>(luabind::type(o)); }
void Lua::CheckType(const luabind::object &o, Type type)
{
	if(GetType(o) == type)
		return;
	TypeError(o, type);
}

void Lua::PushObject(lua::State *l, BaseLuaObj *o) { o->GetLuaObject()->push(l); }

Lua::StatusCode Lua::Execute(lua::State *l, const std::function<Lua::StatusCode(int (*traceback)(lua::State *))> &target)
{
	auto r = target(Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l, r);
	return r;
}

void Lua::Execute(lua::State *, const std::function<void(int (*traceback)(lua::State *), void (*syntaxHandle)(lua::State *, Lua::StatusCode))> &target) { target(Lua::HandleTracebackError, Lua::HandleSyntaxError); }

void Lua::HandleLuaError(lua::State *l)
{
	if(!Lua::IsString(l, -1))
		return;
	std::string msg = Lua::ToString(l, -1);
	msg = ::pragma::scripting::lua_core::format_error_message(l, msg, Lua::StatusCode::ErrorRun);
	::pragma::scripting::lua_core::submit_error(l, msg);
}

void Lua::HandleLuaError(lua::State *l, Lua::StatusCode s)
{
	Lua::HandleTracebackError(l);
	Lua::HandleSyntaxError(l, s);
}

std::string Lua::GetErrorMessagePrefix(lua::State *l)
{
	auto *state = ::pragma::Engine::Get()->GetNetworkState(l);
	if(state != nullptr)
		return state->GetMessagePrefix();
	return "";
}
