/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/console/conout.h>
#include <algorithm>
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
#endif
int Lua::debug::collectgarbage(lua_State *l)
{
	// Calling twice on purpose: https://stackoverflow.com/a/28320364/2482983
	Lua::RunString(l,"collectgarbage()","internal");
	Lua::RunString(l,"collectgarbage()","internal");
	return 0;
}
void Lua::debug::stackdump(lua_State *l)
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
}

void Lua::debug::beep(lua_State *l)
{
	int ms = 500;
	int freq = 523;
#ifdef _WIN32
	Beep(freq,ms);
#else
    //HACK: This should point to console owning pragma
    size_t fd = open("/dev/tty0",O_NONBLOCK | O_RDONLY);
    ioctl(fd,KDMKTONE,(ms<<16 | 1193180 /freq));
    close(fd);
#endif
}

std::string Lua::debug::move_state_to_string(lua_State *l,pragma::BaseAIComponent::MoveResult v)
{
	switch(v)
	{
		case pragma::BaseAIComponent::MoveResult::TargetUnreachable:
			return "ai.MOVE_STATE_TARGET_UNREACHABLE";
		case pragma::BaseAIComponent::MoveResult::TargetReached:
			return "ai.MOVE_STATE_TARGET_REACHED";
		case pragma::BaseAIComponent::MoveResult::WaitingForPath:
			return "ai.MOVE_STATE_WAITING_FOR_PATH";
		case pragma::BaseAIComponent::MoveResult::MovingToTarget:
			return "ai.MOVE_STATE_MOVING_TO_TARGET";
		default:
			return "";
	}
}

void Lua::debug::enable_remote_debugging(lua_State *l)
{
	if(get_extended_lua_modules_enabled() == false)
	{
		Con::cwar<<"Unable to enable remote debugging: Game has to be started with -luaext launch parameter!"<<Con::endl;
		return;
	}
	auto _G = luabind::globals(l);
	auto programPath = util::get_program_path();
	auto path = FileManager::GetNormalizedPath(programPath +"/lua/?.lua");
	path += ";" +FileManager::GetNormalizedPath(programPath +"/lua/modules/?.lua");
#ifdef _WIN32
	std::string ext = ".dll";
#else
	std::string ext = ".so";
#endif
	auto cpath = FileManager::GetNormalizedPath(programPath +"/modules/?" +ext);
	std::replace(path.begin(),path.end(),'\\','/');
	std::replace(cpath.begin(),cpath.end(),'\\','/');
	luabind::object oPackage = _G["package"];
	if(!oPackage)
	{
		Con::cwar<<"Unable to enable remote debugging: package library is missing!"<<Con::endl;
		return;
	}
	oPackage["path"] = path;
	oPackage["cpath"] = cpath;

	Lua::GetGlobal(l,"require");
	Lua::PushString(l,"modules/mobdebug"); // Note: This will disable jit!
	std::string err;
	auto r = Lua::ProtectedCall(l,1,1,&err);
	if(r == Lua::StatusCode::Ok)
	{
		Lua::GetField(l,-1,"start");
		r = Lua::ProtectedCall(l,0,0,&err);
		Lua::Pop(l,1); // Pop return value of "require" from stack
	}
	if(r != Lua::StatusCode::Ok)
		Con::cwar<<"Unable to enable remote debugging: "<<err<<Con::endl;
}
