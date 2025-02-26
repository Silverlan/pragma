/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/baseluaobj.h"
#include <pragma/lua/lua_error_handling.hpp>
#include <stack>
#include <sharedutils/util_file.h>

extern DLLNETWORK Engine *engine;

static void Lua::TypeError(const luabind::object &o, Type type)
{
	// TODO
	auto *l = o.interpreter();
	o.push(l);
	auto *typeName = Lua::GetTypeString(l, -1);
	Lua::Pop(l, 1);
	Lua::PushString(l, typeName);
	lua_error(o.interpreter());
}
Lua::Type Lua::GetType(const luabind::object &o) { return static_cast<Type>(luabind::type(o)); }
void Lua::CheckType(const luabind::object &o, Type type)
{
	if(GetType(o) == type)
		return;
	TypeError(o, type);
}

void Lua::PushObject(lua_State *l, BaseLuaObj *o) { o->GetLuaObject()->push(l); }

Lua::StatusCode Lua::Execute(lua_State *l, const std::function<Lua::StatusCode(int (*traceback)(lua_State *))> &target)
{
	auto r = target(Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l, r);
	return r;
}

void Lua::Execute(lua_State *, const std::function<void(int (*traceback)(lua_State *), void (*syntaxHandle)(lua_State *, Lua::StatusCode))> &target) { target(Lua::HandleTracebackError, Lua::HandleSyntaxError); }

void Lua::HandleLuaError(lua_State *l) { Lua::HandleTracebackError(l); }

void Lua::HandleLuaError(lua_State *l, Lua::StatusCode s)
{
	Lua::HandleTracebackError(l);
	Lua::HandleSyntaxError(l, s);
}

std::string Lua::GetErrorMessagePrefix(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	if(state != nullptr)
		return state->GetMessagePrefix();
	return "";
}
