/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldata.hpp"
#include <pragma/lua/luaapi.h>
#include <datasystem.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>

void Lua::DataBlock::load(lua_State *l,const std::string &fileName)
{
	auto db = ds::System::LoadData(fileName.c_str());
	if(db == nullptr)
		return;
	Lua::Push(l,db);
}
void Lua::DataBlock::load(lua_State *l,VFilePtr f)
{
	auto db = ds::System::ReadData(f);
	if(db == nullptr)
		return;
	Lua::Push(l,db);
}
void Lua::DataBlock::create(lua_State *l)
{
	auto settings = ds::create_data_settings({std::unordered_map<std::string,std::string>{}});
	auto db = std::make_shared<ds::Block>(*settings);
	Lua::Push(l,db);
}
void Lua::DataBlock::GetInt(lua_State *l,ds::Block &data,const std::string &val,int32_t default) {Lua::PushInt(l,data.GetInt(val,default));}
void Lua::DataBlock::GetFloat(lua_State *l,ds::Block &data,const std::string &val,float default) {Lua::PushNumber(l,data.GetFloat(val,default));}
void Lua::DataBlock::GetBool(lua_State *l,ds::Block &data,const std::string &val,bool default) {Lua::PushBool(l,data.GetBool(val,false));}
void Lua::DataBlock::GetString(lua_State *l,ds::Block &data,const std::string &val,const std::string &default) {Lua::PushString(l,data.GetString(val,""));}
void Lua::DataBlock::GetColor(lua_State *l,ds::Block &data,const std::string &key,const Color &default)
{
	auto val = data.GetValue(key);
	if(val == nullptr || typeid(*val) != typeid(ds::Color))
	{
		Lua::Push<Color>(l,default);
		return;
	}
	Lua::Push<Color>(l,static_cast<ds::Color&>(*val).GetValue());
}
void Lua::DataBlock::GetVector(lua_State *l,ds::Block &data,const std::string &key,const Vector3 &default)
{
	auto val = data.GetValue(key);
	if(val == nullptr || typeid(*val) != typeid(ds::Vector))
	{
		Lua::Push<Vector3>(l,default);
		return;
	}
	Lua::Push<Vector3>(l,static_cast<ds::Vector&>(*val).GetValue());
}
void Lua::DataBlock::GetVector4(lua_State *l,ds::Block &data,const std::string &key,const ::Vector4 &default)
{
	auto val = data.GetValue(key);
	if(val == nullptr || typeid(*val) != typeid(ds::Vector4))
	{
		Lua::Push<::Vector4>(l,default);
		return;
	}
	Lua::Push<::Vector4>(l,static_cast<ds::Vector4&>(*val).GetValue());
}
void Lua::DataBlock::GetInt(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushInt(l,data.GetInt(val));}
void Lua::DataBlock::GetFloat(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushNumber(l,data.GetFloat(val));}
void Lua::DataBlock::GetBool(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushBool(l,data.GetBool(val));}
void Lua::DataBlock::GetString(lua_State *l,ds::Block &data,const std::string &val) {Lua::PushString(l,data.GetString(val));}
void Lua::DataBlock::GetColor(lua_State *l,ds::Block &data,const std::string &key) {GetColor(l,data,key,Color::White);}
void Lua::DataBlock::GetVector(lua_State *l,ds::Block &data,const std::string &key) {GetVector(l,data,key,Vector3{});}
void Lua::DataBlock::GetVector4(lua_State *l,ds::Block &data,const std::string &key) {GetVector4(l,data,key,Vector4{});}

void Lua::DataBlock::GetData(lua_State *l,ds::Block &data,const std::string &val)
{
	auto &v = data.GetValue(val);
	if(v == nullptr || !v->IsBlock())
		return;
	auto bl = std::static_pointer_cast<ds::Block>(v);
	Lua::Push<std::shared_ptr<ds::Block>>(l,bl);
}
void Lua::DataBlock::GetChildBlocks(lua_State *l,ds::Block &data)
{
	auto t = Lua::CreateTable(l);
	for(auto &pair : *data.GetData())
	{
		if(pair.second->IsBlock() == false)
			continue;
		Lua::PushString(l,pair.first);
		Lua::Push(l,std::static_pointer_cast<ds::Block>(pair.second));
		Lua::SetTableValue(l,t);
	}
}
void Lua::DataBlock::SetValue(lua_State*,ds::Block &data,const std::string &type,const std::string &key,const std::string &val)
{
	data.AddValue(type,key,val);
}
void Lua::DataBlock::Merge(lua_State *l,ds::Block &data,ds::Block &other)
{
	auto *data0 = const_cast<std::unordered_map<std::string,std::shared_ptr<ds::Base>>*>(data.GetData());
	auto *data1 = other.GetData();
	for(auto &pair : *data1)
		(*data0)[pair.first] = pair.second;
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
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,typeid(*val) == typeid(ds::Color));
}
void Lua::DataBlock::IsVector(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,typeid(*val) == typeid(ds::Vector));
}
void Lua::DataBlock::IsVector4(lua_State *l,ds::Block &data,const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,typeid(*val) == typeid(ds::Vector4));
}
