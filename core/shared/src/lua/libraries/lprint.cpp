#include "stdafx_shared.h"
#include "pragma/lua/libraries/lprint.h"
#include <pragma/console/conout.h>
#include "pragma/lua/ldefinitions.h"
#include <pragma/engine.h>
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

DLLNETWORK int Lua_print(lua_State *L)
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
DLLNETWORK int Lua_PrintTable(lua_State *l,std::string tab,int idx)
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
			Lua_PrintTable(l,tabSub,Lua::GetStackTop(l));
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

DLLNETWORK int Lua_PrintTable(lua_State *l) {return Lua_PrintTable(l,"");}

extern DLLENGINE Engine *engine;
DLLNETWORK int Lua_Msg(lua_State *l,int st)
{
	int argc = lua_gettop(l);
	if(argc > 0)
	{
		NetworkState *state = engine->GetNetworkState(l);
#ifdef _WIN32
		if(state == NULL)
			Con::attr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		else if(state->IsServer())
			Con::attr(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		else
			Con::attr(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
		if(state == NULL)
			std::cout<<"\033[37;1m";
		else if(state->IsServer())
			std::cout<<"\033[36;1m";
		else
			std::cout<<"\033[35;1m";
#endif
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

DLLNETWORK int Lua_Msg(lua_State *l) {return Lua_Msg(l,1);}

DLLNETWORK int Lua_MsgN(lua_State *l)
{
	Lua_Msg(l);
	Con::cout<<Con::endl;
	return 0;
}

DLLNETWORK int Lua_MsgC(lua_State *l)
{
	int flags = Lua::CheckInt<int>(l,1);
	Con::attr(flags);
	Lua_Msg(l,2);
	Con::cout<<Con::endl;
	return 0;
}

DLLNETWORK int Lua_MsgW(lua_State *l)
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

DLLNETWORK int Lua_MsgE(lua_State *l)
{
	int argc = lua_gettop(l);
	if(argc == 0)
		return 0;
	Con::cwar<<"ERROR: ";
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