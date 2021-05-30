/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lprint.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/console/conout.h>
#include "pragma/lua/ldefinitions.h"
#include <pragma/engine.h>
#include <mathutil/color.h>
#include "luasystem.h"

static bool lua_value_to_string(lua_State *L,int arg,int *r,std::string *val)
{
	Lua::PushValue(L,arg);
	arg = Lua::GetStackTop(L);
	lua_getglobal(L, "tostring");
	const char *s;
	size_t l;
	lua_pushvalue(L, -1);  /* function to be called */
	lua_pushvalue(L, arg);   /* value to print */
	lua_call(L, 1, 1);
	s = lua_tolstring(L, -1, &l);  /* get result */
	if (s == NULL)
	{
		*r = luaL_error(L, "'tostring' must return a string to 'print'");
		return false;
	}
	*val = std::string(s,l);
	lua_pop(L, 1);  /* pop result */
	lua_pop(L, 1);  /* pop function */
	lua_pop(L, 1);  /* pop value */
	*r = 0;
	return true;
}

int Lua::console::print(lua_State *L)
{
	/*int argc = lua_gettop(l);
	for(int i=1;i<=argc;i++)
	{
		if(i > 1) Con::cout<<"\t";
		const char *arg = lua_gettype(l,i);
		Con::cout<<arg;
	}
	Con::cout<<Con::endl;
	return 0;*/
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	for (i=1; i<=n; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(L,i,&status,&val) == false)
			return status;
		if (i>1) Con::cout<<"\t";
		Con::cout<<val;
	}
	Con::cout<<Con::endl;
	return 0;
}

// TODO: Prevent infinite loops (e.g. printing _G)
int Lua::console::print_table(lua_State *l,std::string tab,int idx)
{
	Lua::CheckTable(l,idx);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,idx) != 0)
	{
		if(Lua::IsTable(l,-1))
		{
			auto status = -1;
			std::string key;
			if(lua_value_to_string(l,-2,&status,&key) == false)
				return status;
			Con::cout<<tab<<key<<":"<<Con::endl;

			std::string tabSub = tab;
			tabSub += "\t";
			print_table(l,tabSub,Lua::GetStackTop(l));
		}
		else
		{
			auto status = -1;
			std::string key;
			if(lua_value_to_string(l,-2,&status,&key) == false)
				return status;

			status = -1;
			std::string val;
			if(lua_value_to_string(l,-1,&status,&val) == false)
				return status;
			Con::cout<<tab<<key<<" = "<<val<<Con::endl;
		}
		Lua::Pop(l,1);
	}
	return 0;
}

int Lua::console::print_table(lua_State *l) {return print_table(l,"");}

extern DLLNETWORK Engine *engine;
int Lua::console::msg(lua_State *l,int st)
{
	int argc = lua_gettop(l);
	if(argc > 0)
	{
		NetworkState *state = engine->GetNetworkState(l);
		if(state == NULL)
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		else if(state->IsServer())
			util::set_console_color(util::ConsoleColorFlags::Cyan | util::ConsoleColorFlags::Intensity);
		else
			util::set_console_color(util::ConsoleColorFlags::Magenta | util::ConsoleColorFlags::Intensity);
	}
	for(int i=st;i<=argc;i++)
	{
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l,i,&status,&val) == false)
			return status;
		Con::cout<<val;
	}
	return 0;
}

int Lua::debug::print(lua_State *l)
{
	auto flags = util::ConsoleColorFlags::None;
	if(engine->GetNetworkState(l)->IsClient())
		flags |= util::ConsoleColorFlags::BackgroundMagenta;
	else
		flags |= util::ConsoleColorFlags::BackgroundCyan;
	util::set_console_color(flags | util::ConsoleColorFlags::BackgroundIntensity | util::ConsoleColorFlags::Black);
	int n = lua_gettop(l);  /* number of arguments */
	int i;
	for (i=1; i<=n; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l,i,&status,&val) == false)
			return status;
		if (i>1) Con::cout<<"\t";
		Con::cout<<val;
	}
	Con::cout<<Con::endl;
	beep(l);
	return 0;
}

int Lua::console::msg(lua_State *l) {return msg(l,1);}

int Lua::console::msgn(lua_State *l)
{
	msg(l);
	Con::cout<<Con::endl;
	return 0;
}

int Lua::console::msgc(lua_State *l)
{
	if(Lua::IsType<Color>(l,1))
	{
		auto &col = Lua::Check<Color>(l,1);

		auto argc = lua_gettop(l);
		std::stringstream ss {};
		for(int i=2;i<=argc;i++)
		{
			auto status = -1;
			std::string val;
			if(lua_value_to_string(l,i,&status,&val) == false)
				return status;
			ss<<val;
		}

		Con::print(ss.str(),col);
		return 0;
	}
	int flags = Lua::CheckInt<int>(l,1);
	Con::attr(flags);
	msg(l,2);
	Con::cout<<Con::endl;
	return 0;
}

int Lua::console::msgw(lua_State *l)
{
	int argc = lua_gettop(l);
	if(argc == 0)
		return 0;
	Con::cwar<<"WARNING: ";
	for(int i=1;i<=argc;i++)
	{
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l,i,&status,&val) == false)
			return status;
		Con::cwar<<val;
	}
	Con::cwar<<Con::endl;
	return 0;
}

int Lua::console::msge(lua_State *l)
{
	int argc = lua_gettop(l);
	if(argc == 0)
		return 0;
	Con::cerr<<"ERROR: ";
	for(int i=1;i<=argc;i++)
	{
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l,i,&status,&val) == false)
			return status;
		Con::cerr<<val;
	}
	Con::cerr<<Con::endl;
	return 0;
}