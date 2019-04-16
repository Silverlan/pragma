#include "stdafx_shared.h"
#include "pragma/lua/classes/ldata.hpp"
#include <luasystem.h>
#include <datasystem.h>

void Lua::DataBlock::GetInt(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushInt(l,data.GetInt(val));}
void Lua::DataBlock::GetFloat(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushNumber(l,data.GetFloat(val));}
void Lua::DataBlock::GetBool(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushBool(l,data.GetBool(val));}
void Lua::DataBlock::GetString(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushString(l,data.GetString(val));}
void Lua::DataBlock::GetData(lua_State *l,ds::Block &data,const std::string &val)
{
	auto &v = data.GetValue(val);
	if(v == nullptr || !v->IsBlock())
		return;
	auto bl = std::static_pointer_cast<ds::Block>(v);
	Lua::Push<std::shared_ptr<ds::Block>>(l,bl);
}
void Lua::DataBlock::SetValue(lua_State*,ds::Block &data,const std::string &type,const std::string &key,const std::string &val)
{
	data.AddValue(type,key,val);
}
