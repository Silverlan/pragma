#include "stdafx_shared.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/baseluaobj.h"
#include <pragma/lua/lua_error_handling.hpp>
#include <stack>
#include <sharedutils/util_file.h>

extern DLLENGINE Engine *engine;

void Lua::PushObject(lua_State *l,BaseLuaObj *o)
{
	o->GetLuaObject()->push(l);
}

Lua::StatusCode Lua::Execute(lua_State *l,const std::function<Lua::StatusCode(int(*traceback)(lua_State*))> &target,ErrorColorMode colorMode)
{
	auto r = target(Lua::HandleTracebackError);
	Lua::HandleSyntaxError(l,r);
	return r;
}

void Lua::Execute(lua_State*,const std::function<void(int(*traceback)(lua_State*),void(*syntaxHandle)(lua_State*,Lua::StatusCode))> &target,ErrorColorMode colorMode)
{
	target(Lua::HandleTracebackError,Lua::HandleSyntaxError);
}

Lua::StatusCode Lua::Execute(lua_State *l,const std::function<Lua::StatusCode(int(*traceback)(lua_State*))> &target) {return Execute(l,target,GetErrorColorMode(l));}
void Lua::Execute(lua_State *l,const std::function<void(int(*traceback)(lua_State*),void(*syntaxHandle)(lua_State*,Lua::StatusCode))> &target) {Execute(l,target,GetErrorColorMode(l));}

void Lua::HandleLuaError(lua_State *l)
{
	Lua::HandleTracebackError(l);
}

void Lua::HandleLuaError(lua_State *l,Lua::StatusCode s)
{
	Lua::HandleTracebackError(l);
	Lua::HandleSyntaxError(l,s);
}

Lua::ErrorColorMode Lua::GetErrorColorMode(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	if(state != nullptr)
		return state->GetLuaErrorColorMode();
	return ErrorColorMode::White;
}
