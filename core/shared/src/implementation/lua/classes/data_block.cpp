// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.data_block;

void Lua::DataBlock::load(lua::State *l, const std::string &fileName)
{
	auto db = pragma::datasystem::System::LoadData(fileName.c_str());
	if(db == nullptr)
		return;
	Push(l, db);
}
void Lua::DataBlock::load(lua::State *l, LFile &f)
{
	if(!f.IsValid())
		return;
	auto db = pragma::datasystem::System::ReadData(*f.GetHandle());
	if(db == nullptr)
		return;
	Push(l, db);
}
void Lua::DataBlock::create(lua::State *l)
{
	auto settings = pragma::datasystem::create_data_settings({std::unordered_map<std::string, std::string> {}});
	auto db = pragma::util::make_shared<pragma::datasystem::Block>(*settings);
	Push(l, db);
}
void Lua::DataBlock::GetInt(lua::State *l, pragma::datasystem::Block &data, const std::string &val, int32_t def) { PushInt(l, data.GetInt(val, def)); }
void Lua::DataBlock::GetFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &val, float def) { PushNumber(l, data.GetFloat(val, def)); }
void Lua::DataBlock::GetBool(lua::State *l, pragma::datasystem::Block &data, const std::string &val, bool def) { PushBool(l, data.GetBool(val, def)); }
void Lua::DataBlock::GetString(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const std::string &def) { PushString(l, data.GetString(val, def)); }
void Lua::DataBlock::GetColor(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const ::Color &def) { Lua::Push<::Color>(l, data.GetColor(val, def)); }
void Lua::DataBlock::GetVector(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const Vector3 &def) { Lua::Push<Vector3>(l, data.GetVector3(val, def)); }
void Lua::DataBlock::GetVector2(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const ::Vector2 &def) { Lua::Push<::Vector2>(l, data.GetVector2(val, def)); }
void Lua::DataBlock::GetVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &val, const ::Vector4 &def) { Lua::Push<::Vector4>(l, data.GetVector4(val, def)); }
void Lua::DataBlock::GetInt(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { PushInt(l, data.GetInt(val)); }
void Lua::DataBlock::GetFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { PushNumber(l, data.GetFloat(val)); }
void Lua::DataBlock::GetBool(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { PushBool(l, data.GetBool(val)); }
void Lua::DataBlock::GetString(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { PushString(l, data.GetString(val)); }
void Lua::DataBlock::GetColor(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { Lua::Push<::Color>(l, data.GetColor(val)); }
void Lua::DataBlock::GetVector(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { Lua::Push<Vector3>(l, data.GetVector3(val)); }
void Lua::DataBlock::GetVector2(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { Lua::Push<::Vector2>(l, data.GetVector2(val)); }
void Lua::DataBlock::GetVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &val) { Lua::Push<::Vector4>(l, data.GetVector4(val)); }
void Lua::DataBlock::GetValue(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto &v = data.GetValue(key);
	if(!v || v->IsBlock())
		return;
	auto o = nil;
	auto &dsVal = static_cast<pragma::datasystem::Value &>(*v);
	if(typeid(dsVal) == typeid(pragma::datasystem::Int))
		o = luabind::object {l, static_cast<pragma::datasystem::Int &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Float))
		o = luabind::object {l, static_cast<pragma::datasystem::Float &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Bool))
		o = luabind::object {l, static_cast<pragma::datasystem::Bool &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::String))
		o = luabind::object {l, static_cast<pragma::datasystem::String &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Color))
		o = luabind::object {l, static_cast<pragma::datasystem::Color &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Vector))
		o = luabind::object {l, static_cast<pragma::datasystem::Vector &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Vector2))
		o = luabind::object {l, static_cast<pragma::datasystem::Vector2 &>(dsVal).GetValue()};
	else if(typeid(dsVal) == typeid(pragma::datasystem::Vector4))
		o = luabind::object {l, static_cast<pragma::datasystem::Vector4 &>(dsVal).GetValue()};
	o.push(l);
}

void Lua::DataBlock::GetData(lua::State *l, pragma::datasystem::Block &data, const std::string &val)
{
	auto &v = data.GetValue(val);
	if(v == nullptr || !v->IsBlock())
		return;
	auto bl = std::static_pointer_cast<pragma::datasystem::Block>(v);
	Lua::Push<std::shared_ptr<pragma::datasystem::Block>>(l, bl);
}
void Lua::DataBlock::GetChildBlocks(lua::State *l, pragma::datasystem::Block &data)
{
	auto t = CreateTable(l);
	for(auto &pair : *data.GetData()) {
		if(pair.second->IsBlock() == false)
			continue;
		PushString(l, pair.first);
		Push(l, std::static_pointer_cast<pragma::datasystem::Block>(pair.second));
		SetTableValue(l, t);
	}
}
void Lua::DataBlock::SetValue(lua::State *, pragma::datasystem::Block &data, const std::string &type, const std::string &key, const std::string &val) { data.AddValue(type, key, val); }
void Lua::DataBlock::Merge(lua::State *l, pragma::datasystem::Block &data, pragma::datasystem::Block &other)
{
	auto *data0 = const_cast<pragma::datasystem::Block::DataMap *>(data.GetData());
	auto *data1 = other.GetData();
	for(auto &pair : *data1)
		(*data0)[pair.first] = pair.second;
}
void Lua::DataBlock::RemoveValue(lua::State *, pragma::datasystem::Block &data, const std::string &key) { data.RemoveValue(key); }
void Lua::DataBlock::IsEmpty(lua::State *l, pragma::datasystem::Block &data) { PushBool(l, data.IsEmpty()); }
void Lua::DataBlock::HasValue(lua::State *l, pragma::datasystem::Block &data, const std::string &key) { PushBool(l, data.HasValue(key)); }
void Lua::DataBlock::AddBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto block = data.AddBlock(key);
	Push(l, block);
}
void Lua::DataBlock::FindBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto block = data.GetBlock(key);
	if(block == nullptr)
		return;
	Push(l, block);
}
void Lua::DataBlock::FindBlock(lua::State *l, pragma::datasystem::Block &data, const std::string &key, uint32_t blockIndex)
{
	auto block = data.GetBlock(key, blockIndex);
	if(block == nullptr)
		return;
	Push(l, block);
}
void Lua::DataBlock::IsString(lua::State *l, pragma::datasystem::Block &data, const std::string &key) { PushBool(l, data.IsString(key)); }
void Lua::DataBlock::IsInt(lua::State *l, pragma::datasystem::Block &data, const std::string &key) { PushBool(l, data.IsInt(key)); }
void Lua::DataBlock::IsFloat(lua::State *l, pragma::datasystem::Block &data, const std::string &key) { PushBool(l, data.IsFloat(key)); }
void Lua::DataBlock::IsBool(lua::State *l, pragma::datasystem::Block &data, const std::string &key) { PushBool(l, data.IsBool(key)); }
void Lua::DataBlock::IsColor(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		PushBool(l, false);
		return;
	}
	PushBool(l, typeid(*val) == typeid(pragma::datasystem::Color));
}
void Lua::DataBlock::IsVector(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		PushBool(l, false);
		return;
	}
	PushBool(l, typeid(*val) == typeid(pragma::datasystem::Vector));
}
void Lua::DataBlock::IsVector4(lua::State *l, pragma::datasystem::Block &data, const std::string &key)
{
	auto val = data.GetValue(key);
	if(val == nullptr) {
		PushBool(l, false);
		return;
	}
	PushBool(l, typeid(*val) == typeid(pragma::datasystem::Vector4));
}
void Lua::DataBlock::ToString(lua::State *l, pragma::datasystem::Block &data, const std::string &rootIdentifier, uint8_t tabDepth) { PushString(l, data.ToString(rootIdentifier, tabDepth)); }
void Lua::DataBlock::ToString(lua::State *l, pragma::datasystem::Block &data, const std::string &rootIdentifier) { PushString(l, data.ToString(rootIdentifier)); }
void Lua::DataBlock::ToString(lua::State *l, pragma::datasystem::Block &data, uint8_t tabDepth) { PushString(l, data.ToString({}, tabDepth)); }
void Lua::DataBlock::ToString(lua::State *l, pragma::datasystem::Block &data) { PushString(l, data.ToString({})); }
