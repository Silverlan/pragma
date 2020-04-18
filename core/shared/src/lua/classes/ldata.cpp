#include "stdafx_shared.h"
#include "pragma/lua/classes/ldata.hpp"
#include <pragma/lua/luaapi.h>
#include <datasystem.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>

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
void Lua::DataBlock::RemoveValue(lua_State*,ds::Block &data,const std::string &key)
{
	data.RemoveValue(key);
}
void Lua::DataBlock::IsEmpty(lua_State *l,ds::Block &data)
{
	Lua::PushBool(l,data.IsEmpty());
}
void Lua::DataBlock::HasValue(lua_State *l,ds::Block &data,const std::string &key)
{
	Lua::PushBool(l,data.HasValue(key));
}
void Lua::DataBlock::AddBlock(lua_State *l,ds::Block &data,const std::string &key)
{
	auto block = data.AddBlock(key);
	Lua::Push(l,block);
}
void Lua::DataBlock::FindBlock(lua_State *l,ds::Block &data,const std::string &key)
{
	auto block = data.GetBlock(key);
	if(block == nullptr)
		return;
	Lua::Push(l,block);
}
void Lua::DataBlock::FindBlock(lua_State *l,ds::Block &data,const std::string &key,uint32_t blockIndex)
{
	auto block = data.GetBlock(key,blockIndex);
	if(block == nullptr)
		return;
	Lua::Push(l,block);
}
void Lua::DataBlock::IsString(lua_State *l,ds::Block &data,const std::string &key)
{
	Lua::PushBool(l,data.IsString(key));
}
void Lua::DataBlock::IsInt(lua_State *l,ds::Block &data,const std::string &key)
{
	Lua::PushBool(l,data.IsInt(key));
}
void Lua::DataBlock::IsFloat(lua_State *l,ds::Block &data,const std::string &key)
{
	Lua::PushBool(l,data.IsFloat(key));
}
void Lua::DataBlock::IsBool(lua_State *l,ds::Block &data,const std::string &key)
{
	Lua::PushBool(l,data.IsBool(key));
}
void Lua::DataBlock::IsColor(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr)
		return;
	Lua::PushBool(l,typeid(*val) == typeid(ds::Color));
}
void Lua::DataBlock::IsVector(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr)
		return;
	Lua::PushBool(l,typeid(*val) == typeid(ds::Vector));
}
void Lua::DataBlock::GetColor(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr || typeid(*val) != typeid(ds::Color))
		return;
	Lua::Push<Color>(l,static_cast<ds::Color&>(*val).GetValue());
}
void Lua::DataBlock::GetVector(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr || typeid(*val) != typeid(ds::Vector))
		return;
	Lua::Push<Vector3>(l,static_cast<ds::Vector&>(*val).GetValue());
}
