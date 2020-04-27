/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/console/conout.h>
#include "pragma/entities/components/base_ai_component.hpp"

int Lua::debug::stackdump(lua_State *l)
{
	int top = lua_gettop(l);
	Con::cout<<"Total in stack: "<<top<<Con::endl;
	for(int i=1;i<=top;i++)
	{
		int t = lua_type(l,i);
		switch(t)
		{
		case LUA_TSTRING:
			Con::cout<<"string: "<<lua_tostring(l,i)<<Con::endl;
			break;
		case LUA_TBOOLEAN:
			Con::cout<<"boolean: "<<(lua_toboolean(l,i) ? "true" : "false")<<Con::endl;
			break;
		case LUA_TNUMBER:
			Con::cout<<"number: "<<lua_tonumber(l,i)<<Con::endl;
			break;
		default:
			Con::cout<<lua_typename(l,i)<<Con::endl;
			break;
		}
	}
	if(top > 0)
		Con::cout<<Con::endl;
	return 0;
}

int Lua::debug::move_state_to_string(lua_State *l)
{
	auto v = Lua::CheckInt(l,1);
	switch(static_cast<pragma::BaseAIComponent::MoveResult>(v))
	{
		case pragma::BaseAIComponent::MoveResult::TargetUnreachable:
			Lua::PushString(l,"ai.MOVE_STATE_TARGET_UNREACHABLE");
			break;
		case pragma::BaseAIComponent::MoveResult::TargetReached:
			Lua::PushString(l,"ai.MOVE_STATE_TARGET_REACHED");
			break;
		case pragma::BaseAIComponent::MoveResult::WaitingForPath:
			Lua::PushString(l,"ai.MOVE_STATE_WAITING_FOR_PATH");
			break;
		case pragma::BaseAIComponent::MoveResult::MovingToTarget:
			Lua::PushString(l,"ai.MOVE_STATE_MOVING_TO_TARGET");
			break;
		default:
			Lua::PushString(l,"");
			break;
	}
	return 1;
}

int Lua::debug::enable_remote_debugging(lua_State *l)
{
	if(get_extended_lua_modules_enabled() == false)
	{
		Con::cwar<<"WARNING: Unable to enable remote debugging: Game has to be started with -luaext launch parameter!"<<Con::endl;
		return 0;
	}
	auto _G = luabind::globals(l);
	auto programPath = util::get_program_path();
	auto path = FileManager::GetNormalizedPath(programPath +"/lua/?.lua");
	path += ";" +FileManager::GetNormalizedPath(programPath +"/lua/modules/?.lua");
	auto cpath = FileManager::GetNormalizedPath(programPath +"/modules/?.dll");
	luabind::object oPackage = _G["package"];
	if(!oPackage)
	{
		Con::cwar<<"WARNING: Unable to enable remote debugging: package library is missing!"<<Con::endl;
		return 0;
	}
	oPackage["path"] = path;
	oPackage["cpath"] = cpath;

	Lua::GetGlobal(l,"require");
	Lua::PushString(l,"modules/mobdebug");
	std::string err;
	auto r = Lua::ProtectedCall(l,1,1,&err);
	if(r == Lua::StatusCode::Ok)
	{
		Lua::GetField(l,-1,"start");
		r = Lua::ProtectedCall(l,0,0,&err);
		Lua::Pop(l,1); // Pop return value of "require" from stack
	}
	if(r != Lua::StatusCode::Ok)
		Con::cwar<<"WARNING: Unable to enable remote debugging: "<<err<<Con::endl;
	return 0;
}
