/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldata.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <pragma/lua/luaapi.h>
#include <datasystem.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include <fsys/ifile.hpp>

void Lua::DataBlock::load(lua_State *l, const std::string &fileName)
{
	auto db = ds::System::LoadData(fileName.c_str());
	if(db == nullptr)
		return;
	Lua::Push(l, db);
}
void Lua::DataBlock::load(lua_State *l, LFile &f)
{
	if(!f.IsValid())
		return;
	auto db = ds::System::ReadData(*f.GetHandle());
	if(db == nullptr)
		return;
	Lua::Push(l, db);
}
void Lua::DataBlock::create(lua_State *l)
{
	auto settings = ds::create_data_settings({std::unordered_map<std::string, std::string> {}});
	auto db = std::make_shared<ds::Block>(*settings);
	Lua::Push(l, db);
}
void Lua::DataBlock::GetInt(lua_State *l, ds::Block &data, const std::string &val, int32_t def) { Lua::PushInt(l, data.GetInt(val, def)); }
void Lua::DataBlock::GetFloat(lua_State *l, ds::Block &data, const std::string &val, float def) { Lua::PushNumber(l, data.GetFloat(val, def)); }
void Lua::DataBlock::GetBool(lua_State *l, ds::Block &data, const std::string &val, bool def) { Lua::PushBool(l, data.GetBool(val, def)); }
void Lua::DataBlock::GetString(lua_State *l, ds::Block &data, const std::string &val, const std::string &def) { Lua::PushString(l, data.GetString(val, def)); }
void Lua::DataBlock::GetColor(lua_State *l, ds::Block &data, const std::string &val, const Color &def) { Lua::Push<Color>(l, data.GetColor(val, def)); }
void Lua::DataBlock::GetVector(lua_State *l, ds::Block &data, const std::string &val, const Vector3 &def) { Lua::Push<Vector3>(l, data.GetVector3(val, def)); }
void Lua::DataBlock::GetVector2(lua_State *l, ds::Block &data, const std::string &val, const Vector2 &def) { Lua::Push<Vector2>(l, data.GetVector2(val, def)); }
void Lua::DataBlock::GetVector4(lua_State *l, ds::Block &data, const std::string &val, const ::Vector4 &def) { Lua::Push<Vector4>(l, data.GetVector4(val, def)); }
void Lua::DataBlock::GetInt(lua_State *l, ds::Block &data, const std::string &val) { Lua::PushInt(l, data.GetInt(val)); }
void Lua::DataBlock::GetFloat(lua_State *l, ds::Block &data, const std::string &val) { Lua::PushNumber(l, data.GetFloat(val)); }
void Lua::DataBlock::GetBool(lua_State *l, ds::Block &data, const std::string &val) { Lua::PushBool(l, data.GetBool(val)); }
void Lua::DataBlock::GetString(lua_State *l, ds::Block &data, const std::string &val) { Lua::PushString(l, data.GetString(val)); }
void Lua::DataBlock::GetColor(lua_State *l, ds::Block &data, const std::string &val) { Lua::Push<Color>(l, data.GetColor(val)); }
void Lua::DataBlock::GetVector(lua_State *l, ds::Block &data, const std::string &val) { Lua::Push<Vector3>(l, data.GetVector3(val)); }
void Lua::DataBlock::GetVector2(lua_State *l, ds::Block &data, const std::string &val) { Lua::Push<Vector2>(l, data.GetVector2(val)); }
void Lua::DataBlock::GetVector4(lua_State *l, ds::Block &data, const std::string &val) { Lua::Push<Vector4>(l, data.GetVector4(val)); }
void Lua::DataBlock::GetValue(lua_State *l, ds::Block &data, const std::string &key)
{
	auto &v = data.GetValue(key);
	if(!v || v->IsBlock())
		return;
	auto o = Lua::nil;
	auto &dsVal = static_cast<ds::Value &>(*v);
	if(typeid(dsVal) == typeid(ds::Int))
		o = luabind::object {l, static_cast<ds::Int &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Float))
		o = luabind::object {l, static_cast<ds::Float &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Bool))
		o = luabind::object {l, static_cast<ds::Bool &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::String))
		o = luabind::object {l, static_cast<ds::String &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Color))
		o = luabind::object {l, static_cast<ds::Color &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Vector))
		o = luabind::object {l, static_cast<ds::Vector &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Vector2))
		o = luabind::object {l, static_cast<ds::Vector2 &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(ds::Vector4))
		o = luabind::object {l, static_cast<ds::Vector4 &>(dsVal).GetValue()};
	o.push(l);
}

void Lua::DataBlock::GetData(lua_State *l, ds::Block &data, const std::string &val)
{
	auto &v = data.GetValue(val);
	if(v == nullptr || !v->IsBlock())
		return;
	auto bl = std::static_pointer_cast<ds::Block>(v);
	Lua::Push<std::shared_ptr<ds::Block>>(l, bl);
}
void Lua::DataBlock::GetChildBlocks(lua_State *l, ds::Block &data)
{
	auto t = Lua::CreateTable(l);
	for(auto &pair : *data.GetData()) {
		if(pair.second->IsBlock() == false)
			continue;
		Lua::PushString(l, pair.first);
		Lua::Push(l, std::static_pointer_cast<ds::Block>(pair.second));
		Lua::SetTableValue(l, t);
	}
}
void Lua::DataBlock::SetValue(lua_State *, ds::Block &data, const std::string &type, const std::string &key, const std::string &val) { data.AddValue(type, key, val); }
void Lua::DataBlock::Merge(lua_State *l, ds::Block &data, ds::Block &other)
{
	auto *data0 = const_cast<ds::Block::DataMap *>(data.GetData());
	auto *data1 = other.GetData();
	for(auto &pair : *data1)
		(*data0)[pair.first] = pair.second;
}
void Lua::DataBlock::RemoveValue(lua_State *, ds::Block &data, const std::string &key) { data.RemoveValue(key); }
void Lua::DataBlock::IsEmpty(lua_State *l, ds::Block &data) { Lua::PushBool(l, data.IsEmpty()); }
void Lua::DataBlock::HasValue(lua_State *l, ds::Block &data, const std::string &key) { Lua::PushBool(l, data.HasValue(key)); }
void Lua::DataBlock::AddBlock(lua_State *l, ds::Block &data, const std::string &key)
{
	auto block = data.AddBlock(key);
	Lua::Push(l, block);
}
void Lua::DataBlock::FindBlock(lua_State *l, ds::Block &data, const std::string &key)
{
	auto block = data.GetBlock(key);
	if(block == nullptr)
		return;
	Lua::Push(l, block);
}
void Lua::DataBlock::FindBlock(lua_State *l, ds::Block &data, const std::string &key, uint32_t blockIndex)
{
	auto block = data.GetBlock(key, blockIndex);
	if(block == nullptr)
		return;
	Lua::Push(l, block);
}
void Lua::DataBlock::IsString(lua_State *l, ds::Block &data, const std::string &key) { Lua::PushBool(l, data.IsString(key)); }
void Lua::DataBlock::IsInt(lua_State *l, ds::Block &data, const std::string &key) { Lua::PushBool(l, data.IsInt(key)); }
void Lua::DataBlock::IsFloat(lua_State *l, ds::Block &data, const std::string &key) { Lua::PushBool(l, data.IsFloat(key)); }
void Lua::DataBlock::IsBool(lua_State *l, ds::Block &data, const std::string &key) { Lua::PushBool(l, data.IsBool(key)); }
void Lua::DataBlock::IsColor(lua_State *l, ds::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		Lua::PushBool(l, false);
		return;
	}
	Lua::PushBool(l, typeid(*val) == typeid(ds::Color));
}
void Lua::DataBlock::IsVector(lua_State *l, ds::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		Lua::PushBool(l, false);
		return;
	}
	Lua::PushBool(l, typeid(*val) == typeid(ds::Vector));
}
void Lua::DataBlock::IsVector4(lua_State *l, ds::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		Lua::PushBool(l, false);
		return;
	}
	Lua::PushBool(l, typeid(*val) == typeid(ds::Vector4));
}
void Lua::DataBlock::ToString(lua_State *l, ds::Block &data, const std::string &rootIdentifier, uint8_t tabDepth) { Lua::PushString(l, data.ToString(rootIdentifier, tabDepth)); }
void Lua::DataBlock::ToString(lua_State *l, ds::Block &data, const std::string &rootIdentifier) { Lua::PushString(l, data.ToString(rootIdentifier)); }
void Lua::DataBlock::ToString(lua_State *l, ds::Block &data, uint8_t tabDepth) { Lua::PushString(l, data.ToString({}, tabDepth)); }
void Lua::DataBlock::ToString(lua_State *l, ds::Block &data) { Lua::PushString(l, data.ToString({})); }
