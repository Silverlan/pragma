/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/util/util_game.hpp"
#include <sharedutils/util_path.hpp>
#include <luabind/iterator_policy.hpp>
#include <luainterface.hpp>
#include <udm.hpp>
#include <datasystem_vector.h>
#include <datasystem_color.h>
#include <sharedutils/util_file.h>

extern DLLNETWORK Engine *engine;

#pragma optimize("",off)
template<class TPropertyWrapper,class TClass,typename T>
	void prop_set_basic_type_indexed(lua_State *l,TClass &p,uint32_t idx,const T &v)
{
	static_cast<TPropertyWrapper>(p)[idx] = v;
}
	
template<class TPropertyWrapper,class TClass,typename T>
	void prop_set_basic_type(lua_State *l,TClass &p,const std::string &key,const T &v)
{
	static_cast<TPropertyWrapper>(p)[key] = v;
}

static ::udm::Blob data_stream_to_blob(DataStream &ds)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(),ds->GetData(),data.size());
	ds->SetOffset(offset);
	return ::udm::Blob {std::move(data)};
}

static ::udm::BlobLz4 data_stream_to_lz4_blob(DataStream &ds,uint64_t uncompressedSize)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(),ds->GetData(),data.size());
	ds->SetOffset(offset);
	return ::udm::BlobLz4 {std::move(data),uncompressedSize};
}

static ::udm::Utf8String data_stream_to_utf8(DataStream &ds)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(),ds->GetData(),data.size());
	ds->SetOffset(offset);
	return ::udm::Utf8String {std::move(data)};
}

static void set_property_value(lua_State *l,::udm::LinkedPropertyWrapper p,::udm::Type type,luabind::object o,int32_t idx,bool nil=false)
{
	if(nil)
	{
		p = ::udm::Nil{};
		return;
	}
	if(::udm::is_numeric_type(type))
	{
		auto tag = ::udm::get_numeric_tag(type);
		std::visit([l,idx,&p](auto tag){
			using T = decltype(tag)::type;
			if constexpr(std::is_same_v<T,::udm::Half>)
				p = ::udm::Half{static_cast<float>(Lua::CheckNumber(l,idx))};
			else if constexpr(std::is_same_v<T,::udm::Boolean>)
				p = Lua::CheckBool(l,idx);
			else
				p = static_cast<T>(Lua::CheckNumber(l,idx));
		},tag);
	}
	if(::udm::is_generic_type(type))
	{
		auto tag = ::udm::get_generic_tag(type);
		std::visit([l,idx,&p](auto tag) {
			if constexpr(std::is_same_v<decltype(tag)::type,udm::Nil>)
				return;
			else
				p = static_cast<decltype(tag)::type&>(Lua::Check<decltype(tag)::type>(l,idx));
		},tag);
	}
	switch(type)
	{
	case ::udm::Type::String:
		p = static_cast<::udm::String>(Lua::CheckString(l,idx));
		break;
	case ::udm::Type::Utf8String:
		p = data_stream_to_utf8(Lua::Check<DataStream>(l,idx));
		break;
	case ::udm::Type::Reference:
		p = Lua::Check<::udm::Reference>(l,idx);
		break;
	case ::udm::Type::Struct:
		p = Lua::Check<::udm::Struct>(l,idx);
		break;
	case ::udm::Type::Blob:
		p = data_stream_to_blob(Lua::Check<DataStream>(l,idx));
		break;
	case ::udm::Type::BlobLz4:
	{
		Lua::CheckTable(l,idx);
		luabind::table<> t {luabind::from_stack(l,idx)};
		uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1],uint64_t{});
		DataStream ds = luabind::object_cast_nothrow<DataStream>(t[2],DataStream{});
		p = data_stream_to_lz4_blob(ds,uncompressedSize);
		break;
	}
	}
	static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
}
template<typename T>
	static bool is_type(luabind::object &o)
{
	try
	{
		luabind::object_cast<T*>(o);
	}
	catch(const std::exception &e)
	{
		return false;
	}
	return true;
}
static void set_property_value(lua_State *l,::udm::LinkedPropertyWrapper p,luabind::object o)
{
	auto type = luabind::type(o);
	auto udmType = udm::Type::Nil;
	switch(type)
	{
	case LUA_TBOOLEAN:
		udmType = udm::Type::Boolean;
		break;
	case LUA_TSTRING:
		udmType = udm::Type::String;
		break;
	case LUA_TNUMBER:
		udmType = udm::Type::Float;
		break;
	default:
	{
		if(is_type<udm::Vector2>(o))
			udmType = udm::Type::Vector2;
		else if(is_type<udm::Vector3>(o))
			udmType = udm::Type::Vector3;
		else if(is_type<udm::Vector4>(o))
			udmType = udm::Type::Vector4;
		else if(is_type<udm::Quaternion>(o))
			udmType = udm::Type::Quaternion;
		else if(is_type<udm::EulerAngles>(o))
			udmType = udm::Type::EulerAngles;
		else if(is_type<udm::Transform>(o))
			udmType = udm::Type::Transform;
		else if(is_type<udm::ScaledTransform>(o))
			udmType = udm::Type::ScaledTransform;
		else if(is_type<udm::Mat4>(o))
			udmType = udm::Type::Mat4;
		else if(is_type<udm::Mat3x4>(o))
			udmType = udm::Type::Mat3x4;
		else if(is_type<udm::Srgba>(o))
			udmType = udm::Type::Srgba;
		else if(is_type<udm::HdrColor>(o))
			udmType = udm::Type::HdrColor;
		else if(is_type<udm::Property>(o))
		{
			auto prop = luabind::object_cast<udm::PProperty>(o);
			if(!prop)
			{
				p = udm::Nil{};
				return;
			}
			p = prop;
			return;
		}
		break;
	}
	}
	if(udmType == udm::Type::Nil)
		return;
	set_property_value(l,p,udmType,o,3);
}

static luabind::object get_array_values(lua_State *l,::udm::LinkedPropertyWrapper &p,std::optional<::udm::Type> type)
{
	auto t = luabind::newtable(l);
	auto size = p.GetSize();
	auto *pArray = p.GetValuePtr<::udm::Array>();
	if(size == 0 || pArray == nullptr)
		return t;

	if(!type.has_value())
	{
		int32_t idx = 1;
		for(auto prop : p)
			t[idx++] = prop;
		return t;
	}

	auto vs = [&t,pArray,size](auto tag) {
		using T = decltype(tag)::type;
		auto *p = static_cast<T*>(pArray->GetValues());
		for(auto i=decltype(size){0u};i<size;++i)
		{
			t[i +1] = *p;
			++p;
		}
	};
	if(::udm::is_numeric_type(*type))
		std::visit(vs,::udm::get_numeric_tag(*type));
	else if(::udm::is_generic_type(*type))
		std::visit(vs,::udm::get_generic_tag(*type));
	else if(::udm::is_non_trivial_type(*type))
	{
		std::visit([&vs,size,pArray,&t](auto tag) {
			using T = decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T,udm::String>)
				vs(tag);
			else if constexpr(std::is_same_v<T,udm::Element>)
			{
				for(auto i=decltype(size){0u};i<size;++i)
					t[i +1] = (*pArray)[i];
			}
		},::udm::get_non_trivial_tag(*type));
	}
	return t;
}

static luabind::object get_blob_array_values(lua_State *l,::udm::PropertyWrapper &p,::udm::Type type)
{
	auto t = luabind::newtable(l);
	auto vs = [l,&p,&type,&t](auto tag) {
		using T = decltype(tag)::type;
		auto blobData = p.GetBlobData(type);
		auto *ptr = reinterpret_cast<T*>(blobData.data.data());
		auto n = blobData.data.size() /sizeof(T);
		for(auto i=decltype(n){0u};i<n;++i)
			t[i +1] = ptr[i];
	};
	if(::udm::is_numeric_type(type))
		std::visit(vs,::udm::get_numeric_tag(type));
	else if(::udm::is_generic_type(type))
		std::visit(vs,::udm::get_generic_tag(type));
	return t;
}

static void set_blob_array_values(lua_State *l,::udm::PropertyWrapper &p,const std::string &path,::udm::Type type,luabind::table<> t,::udm::Type blobType=::udm::Type::BlobLz4)
{
	if(blobType != ::udm::Type::Blob && blobType != ::udm::Type::BlobLz4)
		return;
	auto n = Lua::GetObjectLength(l,4);
	auto vs = [l,&p,&type,&t,&path,blobType,n](auto tag) {
		using T = decltype(tag)::type;
		if(blobType == ::udm::Type::Blob)
		{
			auto &blob = p.Add(path,udm::Type::Blob).GetValue<::udm::Blob>();
			blob.data.resize(sizeof(T) *n);
			auto *p = reinterpret_cast<T*>(blob.data.data());
			for(auto i=decltype(n){0u};i<n;++i)
			{
				*p = luabind::object_cast_nothrow<T>(t[i +1],T{});
				++p;
			}
			return;
		}
		if constexpr(std::is_same_v<T,bool>)
		{
			// Boolean vectors in c++ are special, so we'll have to handle them separately...
			std::vector<uint8_t> values {};
			values.resize(n);
			for(auto i=decltype(n){0u};i<n;++i)
				values[i] = luabind::object_cast_nothrow<bool>(t[i +1],false);
			p[path] = ::udm::compress_lz4_blob(reinterpret_cast<uint8_t*>(values.data()),values.size() *sizeof(values.front()));
		}
		else
		{
			std::vector<T> values {};
			values.resize(n);
			for(auto i=decltype(n){0u};i<n;++i)
				values[i] = luabind::object_cast_nothrow<T>(t[i +1],T{});
			p[path] = ::udm::compress_lz4_blob(reinterpret_cast<uint8_t*>(values.data()),values.size() *sizeof(values.front()));
		}
	};
	if(::udm::is_numeric_type(type))
		std::visit(vs,::udm::get_numeric_tag(type));
	else if(::udm::is_generic_type(type))
		std::visit(vs,::udm::get_generic_tag(type));
}

static luabind::object get_property_value(lua_State *l,::udm::Type type,void *ptr)
{
	auto vs = [l,ptr](auto tag) {
		using T = decltype(tag)::type;
		if constexpr(std::is_same_v<T,udm::Nil>)
			return luabind::object{};
		else
		{
			auto &v = *static_cast<T*>(ptr);
			if constexpr(std::is_same_v<T,::udm::Half>)
				return luabind::object{l,static_cast<float>(v)};
			return luabind::object{l,v};
		}
	};
	if(::udm::is_numeric_type(type))
		return std::visit(vs,::udm::get_numeric_tag(type));
	if(::udm::is_generic_type(type))
		return std::visit(vs,::udm::get_generic_tag(type));
	switch(type)
	{
	case ::udm::Type::String:
		return luabind::object{l,*static_cast<udm::String*>(ptr)};
	case ::udm::Type::Srgba:
		return luabind::object{l,*static_cast<udm::Srgba*>(ptr)};
	case ::udm::Type::HdrColor:
		return luabind::object{l,*static_cast<udm::HdrColor*>(ptr)};
	case ::udm::Type::Reference:
		return luabind::object{l,static_cast<udm::Reference*>(ptr)};
	case ::udm::Type::Struct:
		return luabind::object{l,static_cast<udm::Struct*>(ptr)};
	case ::udm::Type::Utf8String:
	{
		auto &utf8String = *static_cast<udm::Utf8String*>(ptr);
		DataStream ds {utf8String.data.data(),static_cast<uint32_t>(utf8String.data.size())};
		return luabind::object{l,ds};
	}
	case ::udm::Type::Blob:
	{
		auto &blob = *static_cast<udm::Blob*>(ptr);
		DataStream ds {blob.data.data(),static_cast<uint32_t>(blob.data.size())};
		return luabind::object{l,ds};
	}
	case ::udm::Type::BlobLz4:
	{
		auto &blobLz4 = *static_cast<udm::BlobLz4*>(ptr);
		auto t = luabind::newtable(l);
		t[1] = blobLz4.uncompressedSize;
		t[2] = DataStream{blobLz4.compressedData.data(),static_cast<uint32_t>(blobLz4.compressedData.size())};
		return t;
	}
	}
	static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
	return {};
}
static luabind::object get_property_value(lua_State *l,::udm::PropertyWrapper &val)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type type;
	auto *ptr = val.GetValuePtr(type);
	if(ptr == nullptr)
		return {};
	return get_property_value(l,type,ptr);
}
static luabind::object get_property_value(lua_State *l,::udm::PropertyWrapper &val,::udm::Type type)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type valType;
	auto *ptr = val.GetValuePtr(valType);
	if(ptr == nullptr)
		return {};
	if(valType == type)
		return get_property_value(l,val);
	if(!::udm::is_convertible(valType,type))
		return {};
	auto vs = [l,ptr,&val,type](auto tag) {
		using T = decltype(tag)::type;
		auto v = val.ToValue<T>();
		if(!v.has_value())
			return luabind::object{};
		return get_property_value(l,type,&*v);
	};
	if(::udm::is_numeric_type(type))
		return std::visit(vs,::udm::get_numeric_tag(type));
	if(::udm::is_generic_type(type))
		return std::visit(vs,::udm::get_generic_tag(type));
	return std::visit(vs,::udm::get_non_trivial_tag(type));
}

class LuaUdmArrayIterator
{
public:
	LuaUdmArrayIterator(::udm::PropertyWrapper &prop);

	udm::ArrayIterator<udm::LinkedPropertyWrapper> begin() {return m_property->begin();}
	udm::ArrayIterator<udm::LinkedPropertyWrapper> end() {return m_property->end();}
private:
	::udm::PropertyWrapper *m_property = nullptr;
};

LuaUdmArrayIterator::LuaUdmArrayIterator(::udm::PropertyWrapper &prop)
	: m_property{&prop}
{}

static void data_block_to_udm(ds::Block &dataBlock,udm::LinkedPropertyWrapper &udm)
{
	std::function<void(udm::LinkedPropertyWrapper&,ds::Block&)> dataBlockToUdm = nullptr;
	dataBlockToUdm = [&dataBlockToUdm,&udm](udm::LinkedPropertyWrapper &prop,ds::Block &block) {
		prop.InitializeProperty();

		for(auto &pair : *block.GetData())
		{
			auto &key = pair.first;
			auto &val = pair.second;
			if(val->IsBlock())
			{
				auto &block = static_cast<ds::Block&>(*pair.second);

				auto &children = *block.GetData();
				auto n = children.size();
				uint32_t i = 0;
				auto isArray = true;
				while(i < n)
				{
					if(children.find(std::to_string(i)) == children.end())
					{
						isArray = false;
						break;
					}
					++i;
				}

				if(isArray)
				{
					auto a = prop.AddArray(key,n,udm::Type::String);
					for(auto i=decltype(n){0u};i<n;++i)
					{
						auto it = children.find(std::to_string(i));
						if(it == children.end())
							throw std::runtime_error{"Unknown error"};
						auto &val = it->second;
						if(val->IsBlock() || val->IsContainer())
							throw std::runtime_error{"Unknown error"};
						a[i] = static_cast<const ds::Value&>(*val).GetString();
					}
					continue;
				}

				dataBlockToUdm(prop[key],block);
				continue;
			}
			if(val->IsContainer())
			{
				auto &container = static_cast<ds::Container&>(*pair.second);
				auto &children = container.GetBlocks();
				auto udmChildren = prop.AddArray(key,children.size());
				uint32_t idx = 0;
				for(auto &child : children)
				{
					if(child->IsContainer() || child->IsBlock())
						continue;
					auto *dsValue = dynamic_cast<ds::Value*>(pair.second.get());
					if(dsValue == nullptr)
						continue;
					udmChildren[idx++] = dsValue->GetString();
				}
				udmChildren.Resize(idx);
				continue;
			}
			auto *dsValue = dynamic_cast<ds::Value*>(val.get());
			assert(dsValue);
			if(dsValue)
			{
				auto *dsStr = dynamic_cast<ds::String*>(dsValue);
				if(dsStr)
					prop[key] = dsStr->GetString();
				auto *dsInt = dynamic_cast<ds::Int*>(dsValue);
				if(dsInt)
					prop[key] = dsInt->GetInt();
				auto *dsFloat = dynamic_cast<ds::Float*>(dsValue);
				if(dsFloat)
					prop[key] = dsFloat->GetFloat();
				auto *dsBool = dynamic_cast<ds::Bool*>(dsValue);
				if(dsBool)
					prop[key] = dsBool->GetBool();
				auto *dsVec = dynamic_cast<ds::Vector*>(dsValue);
				if(dsVec)
					prop[key] = dsVec->GetVector();
				auto *dsVec4 = dynamic_cast<ds::Vector4*>(dsValue);
				if(dsVec4)
					prop[key] = dsVec4->GetVector4();
				auto *dsVec2 = dynamic_cast<ds::Vector2*>(dsValue);
				if(dsVec2)
					prop[key] = dsVec2->GetVector2();
				auto *dsTex = dynamic_cast<ds::Texture*>(dsValue);
				if(dsTex)
					udm["textures"][key] = dsTex->GetString();
				auto *dsCol = dynamic_cast<ds::Color*>(dsValue);
				if(dsCol)
				{
					auto col = dsCol->GetColor();
					auto max = std::numeric_limits<udm::Srgba::value_type>::max();
					if(col.r <= max && col.g <= max && col.b <= max && col.a <= max)
						prop[key] = udm::Srgba{static_cast<uint8_t>(col.r),static_cast<uint8_t>(col.g),static_cast<uint8_t>(col.b),static_cast<uint8_t>(col.a)};
					else
						prop[key] = col.ToVector4();
				}
			}
		}
	};
	dataBlockToUdm(udm,dataBlock);
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Element &el) {
	return os<<"UDMElement["<<el.fromProperty<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Reference &ref) {
	return os<<"UDMReference["<<ref.path<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Data &data) {
	return os<<"UDMData["<<data.GetAssetType()<<"]["<<data.GetAssetVersion()<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::PropertyWrapper &prop) {
	auto valid = static_cast<bool>(prop);
	return os<<"UDMPropertyWrapper["<<::udm::enum_type_to_ascii(prop.GetType())<<"]["<<(valid ? "valid" : "invalid")<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::LinkedPropertyWrapper &prop) {
	auto valid = static_cast<bool>(prop);
	return os<<"UDMLinkedPropertyWrapper["<<::udm::enum_type_to_ascii(prop.GetType())<<"]["<<(valid ? "valid" : "invalid")<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::AssetData &assetData) {
	return os<<"UDMAssetData["<<assetData.GetAssetType()<<"]["<<assetData.GetAssetVersion()<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Array &a) {
	return os<<"UDMArray["<<::udm::enum_type_to_ascii(a.GetValueType())<<"]["<<a.GetSize()<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::ArrayLz4 &a) {
	return os<<"UDMArrayLz4["<<::udm::enum_type_to_ascii(a.GetValueType())<<"]["<<a.GetSize()<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Property &prop) {
	return os<<"UDMProperty["<<::udm::enum_type_to_ascii(prop.type)<<"]";
}

static std::ostream &operator<<(std::ostream& os,const ::udm::Srgba &srgba) {
	return os<<srgba[0]<<" "<<srgba[1]<<" "<<srgba[2]<<" "<<srgba[3];
}

static std::ostream &operator<<(std::ostream& os,const ::udm::HdrColor &hdr) {
	return os<<hdr[0]<<" "<<hdr[1]<<" "<<hdr[2]<<" "<<hdr[3];
}

static luabind::object get_children(lua_State *l,::udm::PropertyWrapper &p)
{
	auto t = luabind::newtable(l);
	auto *el = p.GetValuePtr<::udm::Element>();
	if(el == nullptr)
		return t;
	auto *linked = p.GetLinked();
	if(linked)
	{
		for(auto pair : p.ElIt())
			t[std::string{pair.key}] = (*linked)[pair.key];
	}
	else
	{
		for(auto pair : p.ElIt())
			t[std::string{pair.key}] = p[pair.key];
	}
	return t;
}

template<typename T>
	static void set_array_values(udm::PropertyWrapper &p,const std::string &name,::udm::Type type,luabind::table<> t,size_t size,::udm::ArrayType arrayType)
{
	auto &a = p.AddArray(name,size,type,arrayType);
	auto *pVal = static_cast<T*>(a.GetValue<::udm::Array>().GetValues());
	for(auto i=decltype(size){0u};i<size;++i)
	{
		*pVal = luabind::object_cast_nothrow<T>(t[i +1],T{});
		++pVal;
	}
}
static void set_array_values(lua_State *l,udm::PropertyWrapper &p,const std::string &name,::udm::Type type,luabind::table<> t,::udm::ArrayType arrayType=::udm::ArrayType::Raw)
{
	auto size = Lua::GetObjectLength(l,4);
	auto vs = [&t,&name,&p,type,arrayType,size](auto tag) {
		using T = decltype(tag)::type;
		if constexpr(!std::is_same_v<T,::udm::Element> && !std::is_same_v<T,::udm::Utf8String> && !std::is_same_v<T,::udm::Array> && !std::is_same_v<T,::udm::ArrayLz4>)
		{
			if(arrayType == ::udm::ArrayType::Raw)
			{
				if constexpr(::udm::Array::IsValueTypeSupported(::udm::type_to_enum<T>()))
					set_array_values<T>(p,name,type,t,size,arrayType);
			}
			else if constexpr(::udm::ArrayLz4::IsValueTypeSupported(::udm::type_to_enum<T>()))
				set_array_values<T>(p,name,type,t,size,arrayType);
		}
	};
	if(::udm::is_numeric_type(type))
		std::visit(vs,::udm::get_numeric_tag(type));
	else if(::udm::is_generic_type(type))
		std::visit(vs,::udm::get_generic_tag(type));
	else if(::udm::is_non_trivial_type(type))
		std::visit(vs,::udm::get_non_trivial_tag(type));
}

template<class T,class TPropertyWrapper,class TClassDef>
	void register_property_methods(TClassDef &classDef)
{
	classDef.def("It",static_cast<LuaUdmArrayIterator(*)(lua_State*,T&)>([](lua_State *l,T &p) -> LuaUdmArrayIterator {
		return LuaUdmArrayIterator{static_cast<TPropertyWrapper>(p)};
	}),luabind::return_stl_iterator{})
	.def("Add",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &path) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).Add(path);
	}))
	.def("Add",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&,::udm::Type)>([](lua_State *l,T &p,const std::string &path,::udm::Type type) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).Add(path,type);
	}))
	.def("AddAssetData",static_cast<::udm::AssetData(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &path) -> ::udm::AssetData {
		return ::udm::AssetData{static_cast<TPropertyWrapper>(p).Add(path)};
	}))
	.def("ToAssetData",static_cast<::udm::AssetData(*)(lua_State*,T&)>([](lua_State *l,T &prop) -> ::udm::AssetData {
		return ::udm::AssetData {static_cast<TPropertyWrapper>(prop)};
	}))
	.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p)[key];
	}))
	.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,uint32_t)>([](lua_State *l,T &p,uint32_t idx) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p)[idx];
	}))
	.def("GetArrayValuesFromBlob",static_cast<luabind::object(*)(lua_State*,T&,::udm::Type)>([](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		return get_blob_array_values(l,static_cast<TPropertyWrapper>(p),type);
	}))
	.def("AddBlobFromArrayValues",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type,luabind::table<>,::udm::Type)>(
		[](lua_State *l,T &p,const std::string &path,::udm::Type type,luabind::table<> t,::udm::Type blobType) {
		return set_blob_array_values(l,static_cast<TPropertyWrapper>(p),path,type,t,blobType);
	}))
	.def("AddBlobFromArrayValues",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type,luabind::table<>)>(
		[](lua_State *l,T &p,const std::string &path,::udm::Type type,luabind::table<> t) {
		return set_blob_array_values(l,static_cast<TPropertyWrapper>(p),path,type,t);
	}))
	.def("GetBlobData",static_cast<void(*)(lua_State*,T&)>([](lua_State *l,T &p) {
		auto type = ::udm::Type::Nil;
		auto vs = [l,&p,&type](auto tag) {
			using T = decltype(tag)::type;
			return static_cast<TPropertyWrapper>(p).GetBlobData(type);
		};
		::udm::Blob blob;
		if(::udm::is_numeric_type(type))
			blob = std::visit(vs,::udm::get_numeric_tag(type));
		else if(::udm::is_generic_type(type))
			blob = std::visit(vs,::udm::get_generic_tag(type));
		else if(::udm::is_non_trivial_type(type))
			return; // TODO
		DataStream ds {static_cast<uint32_t>(blob.data.size())};
		ds->Write(blob.data.data(),blob.data.size());
		ds->SetOffset(0);
		Lua::Push(l,ds);
		Lua::Push(l,type);
	}))
	.def("GetSize",static_cast<uint32_t(*)(lua_State*,T&)>([](lua_State *l,T &p) -> uint32_t {
		return static_cast<TPropertyWrapper>(p).GetSize();
	}))
	.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,T&)>([](lua_State *l,T &p) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,*lp,{});
		return get_array_values(l,::udm::LinkedPropertyWrapper{tp},{});
	}))
	.def("GetValueType",static_cast<luabind::object(*)(lua_State*,T&)>([](lua_State *l,T &p) -> luabind::object {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a)
			return luabind::object{};
		return luabind::object{l,umath::to_integral(a->GetValueType())};
	}))
	.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,T&,::udm::Type)>([](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,*lp,type);
		return get_array_values(l,::udm::LinkedPropertyWrapper{tp},type);
	}))
	.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,T&,const std::string&,::udm::Type)>([](lua_State *l,T &p,const std::string &name,::udm::Type type) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,(*lp)[name],type);
		return get_array_values(l,tp[name],type);
	}))
	.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &name) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,(*lp)[name],{});
		return get_array_values(l,tp[name],{});
	}))
	.def("SetArrayValues",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type,luabind::table<>)>([](lua_State *l,T &p,const std::string &name,::udm::Type type,luabind::table<> t) {
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp,name,type,t);
	}))
	.def("SetArrayValues",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type,luabind::table<>,::udm::Type)>([](lua_State *l,T &p,const std::string &name,::udm::Type type,luabind::table<> t,::udm::Type arrayType) {
		if(arrayType != ::udm::Type::Array && arrayType != ::udm::Type::ArrayLz4)
			Lua::Error(l,"Invalid array type '" +std::string{::udm::enum_type_to_ascii(arrayType)} +"'!");
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp,name,type,t,(arrayType == ::udm::Type::ArrayLz4) ? ::udm::ArrayType::Compressed : ::udm::ArrayType::Raw);
	}))
	.def("GetChildren",static_cast<luabind::object(*)(lua_State*,T&)>([](lua_State *l,T &p) -> luabind::object {
		return get_children(l,static_cast<TPropertyWrapper>(p));
	}))
	.def("GetChildren",static_cast<luabind::object(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &key) -> luabind::object {
		return get_children(l,static_cast<TPropertyWrapper>(p)[key]);
	}))
	.def("GetChildCount",static_cast<uint32_t(*)(lua_State*,T&)>([](lua_State *l,T &p) -> uint32_t {
		return static_cast<TPropertyWrapper>(p).GetChildCount();
	}))
	.def("GetChildCount",static_cast<uint32_t(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &key) -> uint32_t {
		return static_cast<TPropertyWrapper>(p)[key].GetChildCount();
	}))
	.def("Resize",static_cast<void(*)(lua_State*,T&,uint32_t)>([](lua_State *l,T &p,uint32_t size) {
		static_cast<TPropertyWrapper>(p).Resize(size);
	}))
	.def("SetValue",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type,luabind::object)>([](lua_State *l,T &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],type,o,4);
	}))
	.def("SetValue",static_cast<void(*)(lua_State*,T&,uint32_t,::udm::Type,luabind::object)>([](lua_State *l,T &el,uint32_t idx,::udm::Type type,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[idx],type,o,4);
	}))
	.def("SetValue",static_cast<void(*)(lua_State*,T&,const std::string&,::udm::Type)>([](lua_State *l,T &el,const std::string &key,::udm::Type type) {
		if(type != ::udm::Type::Nil)
			Lua::Error(l,"Omitting value only allowed for nil type!");
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],type,luabind::object{},4,true);
	}))
	.def("SetValue",static_cast<void(*)(lua_State*,T&,const std::string&,luabind::object)>([](lua_State *l,T &el,const std::string &key,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],o);
	}))
	.def("HasValue",static_cast<bool(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &key) -> bool {
		return static_cast<bool>(static_cast<TPropertyWrapper>(p)[key]);
	}))
	.def("GetValue",static_cast<luabind::object(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &key) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p)[key]);
	}))
	.def("GetValue",static_cast<luabind::object(*)(lua_State*,T&,const std::string&,::udm::Type)>([](lua_State *l,T &p,const std::string &key,::udm::Type type) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p)[key],type);
	}))
	.def("GetValue",static_cast<luabind::object(*)(lua_State*,T&)>([](lua_State *l,T &p) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p));
	}))
	.def("GetValue",static_cast<luabind::object(*)(lua_State*,T&,::udm::Type)>([](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p),type);
	}))
	.def("GetType",static_cast<::udm::Type(*)(lua_State*,T&)>([](lua_State *l,T &prop) -> ::udm::Type {
		return static_cast<TPropertyWrapper>(prop).GetType();
	}))
	.def("Merge",static_cast<void(*)(lua_State*,T&,::udm::PropertyWrapper&)>([](lua_State *l,T &prop,::udm::PropertyWrapper &propOther) {
		static_cast<TPropertyWrapper>(prop).Merge(propOther);
	}))
	.def("ToAscii",static_cast<luabind::object(*)(lua_State*,T&)>([](lua_State *l,T &prop) -> luabind::object {
		auto *el = static_cast<TPropertyWrapper>(prop).GetValuePtr<::udm::Element>();
		if(!el)
			return {};
		std::stringstream ss;
		el->ToAscii(::udm::AsciiSaveFlags::DontCompressLz4Arrays,ss);
		return luabind::object{l,ss.str()};
	}))
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,bool>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,float>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Vector2>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Vector3>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Vector4>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Quat>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,EulerAngles>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,umath::Transform>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,umath::ScaledTransform>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Mat4>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,Mat3x4>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,std::string>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,::udm::Srgba>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,::udm::HdrColor>)
	.def("Set",&prop_set_basic_type_indexed<TPropertyWrapper,T,::udm::Reference>)

	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,bool>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,float>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Vector2>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Vector3>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Vector4>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Quat>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,EulerAngles>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,umath::Transform>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,umath::ScaledTransform>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Mat4>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,Mat3x4>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,std::string>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,::udm::Srgba>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,::udm::HdrColor>)
	.def("Set",&prop_set_basic_type<TPropertyWrapper,T,::udm::Reference>)
	.def("Set",static_cast<void(*)(lua_State*,T&,uint32_t,DataStream&)>([](lua_State *l,T &p,uint32_t idx,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		static_cast<TPropertyWrapper>(p)[idx] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	}))
	.def("Set",static_cast<void(*)(lua_State*,T&,const std::string&,DataStream&)>([](lua_State *l,T &p,const std::string &key,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		static_cast<TPropertyWrapper>(p)[key] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	}))
	.def("AddArray",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&,uint32_t)>([](lua_State *l,T &p,const std::string &name,uint32_t size) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,::udm::Type::Element);
	}))
	.def("AddArray",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&,uint32_t,::udm::Type)>([](lua_State *l,T &p,const std::string &name,uint32_t size,::udm::Type type) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,type);
	}))
	.def("AddArray",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&,uint32_t,::udm::Type,::udm::ArrayType)>(
		[](lua_State *l,T &p,const std::string &name,uint32_t size,::udm::Type type,::udm::ArrayType arrayType) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,type,arrayType);
	}))
	.def("GetFromPath",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&)>([](lua_State *l,T &p,const std::string &path) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).GetFromPath(path);
	}))
	.def("IsValid",static_cast<bool(*)(lua_State*,T&)>([](lua_State *l,T &prop) -> bool {
		return static_cast<bool>(static_cast<TPropertyWrapper>(prop));
	}));
}

void Lua::udm::register_library(Lua::Interface &lua)
{
	auto &modUdm = lua.RegisterLibrary("udm",std::unordered_map<std::string,int(*)(lua_State*)>{
		{"load",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			if(Lua::IsString(l,1))
			{
				std::string fileName = Lua::CheckString(l,1);
				std::string err;
				auto udmData = ::util::load_udm_asset(fileName,&err);
				if(udmData == nullptr)
				{
					Lua::PushBool(l,false);
					Lua::PushString(l,err);
					return 2;
				}
				Lua::Push(l,udmData);
				return 1;
			}
			auto &f = Lua::Check<LFile>(l,1);
			if(!f.GetHandle())
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,"Invalid file handle!");
				return 2;
			}
			std::string err;
			auto udmData = ::util::load_udm_asset(f.GetHandle(),&err);
			if(udmData == nullptr)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,err);
				return 2;
			}
			Lua::Push(l,udmData);
			return 1;
		})},
		{"open",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			if(Lua::IsString(l,1))
			{
				std::string fileName = Lua::CheckString(l,1);
				try
				{
					auto udmData = ::udm::Data::Open(fileName);
					Lua::Push(l,udmData);
				}
				catch(const ::udm::Exception &e)
				{
					Lua::PushBool(l,false);
					Lua::PushString(l,e.what());
					return 2;
				}
				return 1;
			}
			auto &f = Lua::Check<LFile>(l,1);
			try
			{
				auto udmData = ::udm::Data::Open(f.GetHandle());
				Lua::Push(l,udmData);
			}
			catch(const ::udm::Exception &e)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,e.what());
				return 2;
			}
			return 1;
		})},
		{"create",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string assetType = "";
			uint32_t assetVersion = 1;
			::udm::PProperty rootProp = nullptr;
			int32_t argIdx = 1;
			if(Lua::IsSet(l,argIdx) && Lua::IsType<::udm::Property>(l,argIdx))
				rootProp = Lua::Check<::udm::PProperty>(l,argIdx++);
			if(Lua::IsSet(l,argIdx))
				assetType = Lua::CheckString(l,argIdx++);
			if(Lua::IsSet(l,argIdx))
				assetVersion = Lua::CheckInt(l,argIdx++);

			try
			{
				auto udmData = ::udm::Data::Create(assetType,assetVersion);
				if(rootProp)
					udmData->GetAssetData().GetData() = rootProp;
				Lua::Push(l,udmData);
			}
			catch(const ::udm::Exception &e)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,e.what());
				return 2;
			}
			return 1;
		})},
		{"create_property",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto type = static_cast<::udm::Type>(Lua::CheckInt(l,1));
			auto prop = ::udm::Property::Create(type);
			if(!prop)
				return 0;
			Lua::Push(l,prop);
			return 1;
		})},
		{"create_element",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto type = ::udm::Type::Element;
			auto prop = ::udm::Property::Create(type);
			if(!prop)
				return 0;
			Lua::Push(l,prop);
			return 1;
		})},
		{"compress_lz4",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto &ds = Lua::Check<DataStream>(l,1);
			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto lz4Blob = ::udm::compress_lz4_blob(ds->GetData(),ds->GetInternalSize());
			ds->SetOffset(offset);

			auto t = luabind::newtable(l);
			t[1] = lz4Blob.uncompressedSize;
			DataStream dsCompressed {static_cast<uint32_t>(lz4Blob.compressedData.size())};
			dsCompressed->Write(lz4Blob.compressedData.data(),lz4Blob.compressedData.size());
			t[2] = dsCompressed;
			t.push(l);
			return 1;
		})},
		{"decompress_lz4",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			Lua::CheckTable(l,1);
			auto t = luabind::table<>{luabind::from_stack(l,1)};
			uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1],uint64_t{});
			DataStream ds = luabind::object_cast_nothrow<DataStream>(t[2],DataStream{});

			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto blob = ::udm::decompress_lz4_blob(ds->GetData(),ds->GetInternalSize(),uncompressedSize);
			ds->SetOffset(offset);

			DataStream dsDecompressed {static_cast<uint32_t>(blob.data.size())};
			dsDecompressed->Write(blob.data.data(),blob.data.size());
			Lua::Push(l,dsDecompressed);
			return 1;
		})},
		{"data_block_to_udm",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto &dataBlock = Lua::Check<ds::Block>(l,1);
			auto &udm = Lua::Check<::udm::LinkedPropertyWrapper>(l,2);
			data_block_to_udm(dataBlock,udm);
			return 0;
		})},
		{"data_file_to_udm",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string fileName = Lua::CheckString(l,1);
			std::string rpath;
			if(FileManager::FindAbsolutePath(fileName,rpath) == false)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			auto f = FileManager::OpenFile(fileName.c_str(),"r");
			if(f == nullptr)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			auto root = ds::System::ReadData(f);
			if(root == nullptr)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			ufile::remove_extension_from_filename(rpath);
			rpath += ".udm";
			auto p = util::Path::CreateFile(rpath);
			p.MakeRelative(util::get_program_path());
			rpath = p.GetString();
			auto fout = FileManager::OpenFile<VFilePtrReal>(rpath.c_str(),"w");
			if(fout == nullptr)
			{
				Lua::PushBool(l,false);
				return 1;
			}
			auto udmData = ::udm::Data::Create();
			data_block_to_udm(*root,udmData->GetAssetData().GetData());
			Lua::PushBool(l,udmData->SaveAscii(fout));
			return 1;
		})},
		{"debug_test",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			::udm::Data::DebugTest();
			return 0;
		})},
		{"define_struct",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto t = luabind::table{luabind::from_stack(l,1)};
			::udm::StructDescription structDesc {};
			auto n = Lua::GetObjectLength(l,1);
			structDesc.types.resize(n);
			structDesc.names.resize(n);
			for(auto i=decltype(n){0u};i<n;++i)
			{
				auto o = t[i +1];
				structDesc.types[i] = luabind::object_cast<::udm::Type>(o["type"]);
				structDesc.names[i] = luabind::object_cast<std::string>(o["name"]);
			}
			Lua::Push(l,structDesc);
			return 1;
		})},
		{"enum_type_to_ascii",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto type = Lua::CheckInt(l,1);
			Lua::PushString(l,::udm::enum_type_to_ascii(static_cast<::udm::Type>(type)));
			return 1;
		})},
		{"ascii_type_to_enum",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string stype = Lua::CheckString(l,1);
			auto type = ::udm::ascii_type_to_enum(stype);
			Lua::PushInt(l,umath::to_integral(type));
			return 1;
		})}
	});

	Lua::RegisterLibraryEnums(lua.GetState(),"udm",{
        {"TYPE_NIL",umath::to_integral(::udm::Type::Nil)},
        {"TYPE_STRING",umath::to_integral(::udm::Type::String)},
        {"TYPE_UTF8_STRING",umath::to_integral(::udm::Type::Utf8String)},
        {"TYPE_INT8",umath::to_integral(::udm::Type::Int8)},
        {"TYPE_UINT8",umath::to_integral(::udm::Type::UInt8)},
        {"TYPE_INT16",umath::to_integral(::udm::Type::Int16)},
        {"TYPE_UINT16",umath::to_integral(::udm::Type::UInt16)},
        {"TYPE_INT32",umath::to_integral(::udm::Type::Int32)},
        {"TYPE_UINT32",umath::to_integral(::udm::Type::UInt32)},
        {"TYPE_INT64",umath::to_integral(::udm::Type::Int64)},
        {"TYPE_UINT64",umath::to_integral(::udm::Type::UInt64)},
        {"TYPE_FLOAT",umath::to_integral(::udm::Type::Float)},
        {"TYPE_DOUBLE",umath::to_integral(::udm::Type::Double)},
        {"TYPE_BOOLEAN",umath::to_integral(::udm::Type::Boolean)},
        {"TYPE_VECTOR2",umath::to_integral(::udm::Type::Vector2)},
        {"TYPE_VECTOR2I",umath::to_integral(::udm::Type::Vector2i)},
        {"TYPE_VECTOR3",umath::to_integral(::udm::Type::Vector3)},
        {"TYPE_VECTOR3I",umath::to_integral(::udm::Type::Vector3i)},
        {"TYPE_VECTOR4",umath::to_integral(::udm::Type::Vector4)},
        {"TYPE_VECTOR4I",umath::to_integral(::udm::Type::Vector4i)},
        {"TYPE_QUATERNION",umath::to_integral(::udm::Type::Quaternion)},
        {"TYPE_EULER_ANGLES",umath::to_integral(::udm::Type::EulerAngles)},
        {"TYPE_SRGBA",umath::to_integral(::udm::Type::Srgba)},
        {"TYPE_HDR_COLOR",umath::to_integral(::udm::Type::HdrColor)},
        {"TYPE_TRANSFORM",umath::to_integral(::udm::Type::Transform)},
        {"TYPE_SCALED_TRANSFORM",umath::to_integral(::udm::Type::ScaledTransform)},
        {"TYPE_MAT4",umath::to_integral(::udm::Type::Mat4)},
        {"TYPE_MAT3X4",umath::to_integral(::udm::Type::Mat3x4)},
        {"TYPE_BLOB",umath::to_integral(::udm::Type::Blob)},
        {"TYPE_BLOB_LZ4",umath::to_integral(::udm::Type::BlobLz4)},
        {"TYPE_ELEMENT",umath::to_integral(::udm::Type::Element)},
        {"TYPE_ARRAY",umath::to_integral(::udm::Type::Array)},
        {"TYPE_ARRAY_LZ4",umath::to_integral(::udm::Type::ArrayLz4)},
		{"TYPE_REFERENCE",umath::to_integral(::udm::Type::Reference)},
		{"TYPE_STRUCT",umath::to_integral(::udm::Type::Struct)},
		{"TYPE_HALF",umath::to_integral(::udm::Type::Half)},
        {"TYPE_COUNT",umath::to_integral(::udm::Type::Count)},

        {"ARRAY_TYPE_COMPRESSED",umath::to_integral(::udm::ArrayType::Compressed)},
        {"ARRAY_TYPE_RAW",umath::to_integral(::udm::ArrayType::Raw)},
		
        {"ASCII_SAVE_FLAG_BIT_INCLUDE_HEADER",umath::to_integral(::udm::AsciiSaveFlags::IncludeHeader)},
        {"ASCII_SAVE_FLAG_BIT_DONT_COMPRESS_LZ4_ARRAYS",umath::to_integral(::udm::AsciiSaveFlags::DontCompressLz4Arrays)}
	});
	static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this list when types have been added or removed!");

	auto cdEl = luabind::class_<::udm::Element>("Element");
	cdEl.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::Element,::udm::PropertyWrapper&>(cdEl);
	modUdm[cdEl];
	
	auto cdRef = luabind::class_<::udm::Reference>("Reference");
	cdRef.def(luabind::constructor<>());
	cdRef.def(luabind::constructor<const std::string&>());
	cdRef.def(luabind::tostring(luabind::self));
	cdRef.def("GetProperty",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::Reference&)>([](lua_State *l,::udm::Reference &ref) -> ::udm::PropertyWrapper {
		return ref.property ? ::udm::PropertyWrapper{*ref.property} : ::udm::PropertyWrapper{};
	}));
	modUdm[cdRef];
	
	auto cdStructDescription = luabind::class_<::udm::StructDescription>("StructDescription");
	cdStructDescription.def(luabind::constructor<>());
	cdStructDescription.def("GetTypes",static_cast<luabind::object(*)(lua_State*,::udm::StructDescription&)>([](lua_State *l,::udm::StructDescription &udmData) -> luabind::object {
		return Lua::vector_to_table<::udm::Type>(l,udmData.types);
	}));
	cdStructDescription.def("GetNames",static_cast<luabind::object(*)(lua_State*,::udm::StructDescription&)>([](lua_State *l,::udm::StructDescription &udmData) -> luabind::object {
		return Lua::vector_to_table<std::string>(l,udmData.names);
	}));
	cdStructDescription.def("GetMemberCount",static_cast<uint32_t(*)(lua_State*,::udm::StructDescription&)>([](lua_State *l,::udm::StructDescription &udmData) -> uint32_t {
		return udmData.GetMemberCount();
	}));
	modUdm[cdStructDescription];
	
	auto cdStruct = luabind::class_<::udm::Struct>("Struct");
	cdStruct.def(luabind::constructor<>());
	cdStruct.def(luabind::constructor<const ::udm::StructDescription&>());
	cdStruct.def("GetDescription",static_cast<::udm::StructDescription*(*)(lua_State*,::udm::Struct&)>([](lua_State *l,::udm::Struct &strct) -> ::udm::StructDescription* {
		return &strct.description;
	}));
	cdStruct.def("GetValue",static_cast<luabind::object(*)(lua_State*,::udm::Struct&)>([](lua_State *l,::udm::Struct &strct) -> luabind::object {
		auto t = luabind::newtable(l);
		auto n = strct.description.types.size();
		auto *ptr = strct.data.data();
		auto vs = [l,&t,&ptr](auto tag) {
			using T = decltype(tag)::type;
			if constexpr(std::is_same_v<T,::udm::Nil>)
				return luabind::object{};
			else
			{
				auto &v = *reinterpret_cast<T*>(ptr);
				if constexpr(std::is_same_v<T,::udm::Half>)
					return luabind::object{l,static_cast<float>(v)};
				return luabind::object{l,v};
			}
		};
		for(auto i=decltype(n){0u};i<n;++i)
		{
			t[i +1] = std::visit(vs,::udm::get_generic_tag(strct.description.types[i]));
			ptr += ::udm::size_of(strct.description.types[i]);
		}
		return t;
	}));
	modUdm[cdStruct];

	auto cdSrgba = luabind::class_<::udm::Srgba>("Srgba");
	cdSrgba.def(luabind::constructor<>());
	cdSrgba.def(luabind::tostring(luabind::self));
	cdSrgba.property("r",static_cast<uint8_t(*)(lua_State*,::udm::Srgba&)>([](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[0];
	}),static_cast<void(*)(lua_State*,::udm::Srgba&,uint8_t)>([](lua_State *l,::udm::Srgba &srgba,uint8_t r) {
		srgba[0] = r;
	}));
	cdSrgba.property("g",static_cast<uint8_t(*)(lua_State*,::udm::Srgba&)>([](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[1];
	}),static_cast<void(*)(lua_State*,::udm::Srgba&,uint8_t)>([](lua_State *l,::udm::Srgba &srgba,uint8_t g) {
		srgba[1] = g;
	}));
	cdSrgba.property("b",static_cast<uint8_t(*)(lua_State*,::udm::Srgba&)>([](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[2];
	}),static_cast<void(*)(lua_State*,::udm::Srgba&,uint8_t)>([](lua_State *l,::udm::Srgba &srgba,uint8_t b) {
		srgba[2] = b;
	}));
	cdSrgba.property("a",static_cast<uint8_t(*)(lua_State*,::udm::Srgba&)>([](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[3];
	}),static_cast<void(*)(lua_State*,::udm::Srgba&,uint8_t)>([](lua_State *l,::udm::Srgba &srgba,uint8_t a) {
		srgba[3] = a;
	}));
	modUdm[cdSrgba];

	auto cdHdr = luabind::class_<::udm::HdrColor>("HdrColor");
	cdHdr.def(luabind::constructor<>());
	cdHdr.def(luabind::tostring(luabind::self));
	cdHdr.property("r",static_cast<uint8_t(*)(lua_State*,::udm::HdrColor&)>([](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[0];
	}),static_cast<void(*)(lua_State*,::udm::HdrColor&,uint8_t)>([](lua_State *l,::udm::HdrColor &srgba,uint8_t r) {
		srgba[0] = r;
	}));
	cdHdr.property("g",static_cast<uint8_t(*)(lua_State*,::udm::HdrColor&)>([](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[1];
	}),static_cast<void(*)(lua_State*,::udm::HdrColor&,uint8_t)>([](lua_State *l,::udm::HdrColor &srgba,uint8_t g) {
		srgba[1] = g;
	}));
	cdHdr.property("b",static_cast<uint8_t(*)(lua_State*,::udm::HdrColor&)>([](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[2];
	}),static_cast<void(*)(lua_State*,::udm::HdrColor&,uint8_t)>([](lua_State *l,::udm::HdrColor &srgba,uint8_t b) {
		srgba[2] = b;
	}));
	cdHdr.property("a",static_cast<uint8_t(*)(lua_State*,::udm::HdrColor&)>([](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[3];
	}),static_cast<void(*)(lua_State*,::udm::HdrColor&,uint8_t)>([](lua_State *l,::udm::HdrColor &srgba,uint8_t a) {
		srgba[3] = a;
	}));
	modUdm[cdHdr];

	auto cdData = luabind::class_<::udm::Data>("Data");
	cdData.def(luabind::tostring(luabind::self));
	cdData.def("Save",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &fileName) {
		try
		{
			udmData.Save(fileName);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &fileName) {
		try
		{
			udmData.SaveAscii(fileName);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&,::udm::AsciiSaveFlags)>([](lua_State *l,::udm::Data &udmData,const std::string &fileName,::udm::AsciiSaveFlags flags) {
		try
		{
			udmData.SaveAscii(fileName,flags);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("Save",static_cast<void(*)(lua_State*,::udm::Data&,LFile&)>([](lua_State *l,::udm::Data &udmData,LFile &f) {
		auto fptr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
		if(fptr == nullptr)
			return;
		try
		{
			udmData.Save(fptr);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,LFile&)>([](lua_State *l,::udm::Data &udmData,LFile &f) {
		auto fptr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
		if(fptr == nullptr)
			return;
		try
		{
			udmData.SaveAscii(fptr);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,LFile&,::udm::AsciiSaveFlags)>([](lua_State *l,::udm::Data &udmData,LFile &f,::udm::AsciiSaveFlags flags) {
		auto fptr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
		if(fptr == nullptr)
			return;
		try
		{
			udmData.SaveAscii(fptr,flags);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("ToAscii",static_cast<std::string(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> std::string {
		std::stringstream ss;
		udmData.ToAscii(ss);
		return ss.str();
	}));
	cdData.def("GetAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> ::udm::AssetData {return udmData.GetAssetData();}));
	cdData.def("LoadProperty",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &path) {
		try
		{
			auto prop = udmData.LoadProperty(path);
			Lua::Push(l,prop);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	}));
	cdData.def("GetRootElement",static_cast<::udm::Element*(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> ::udm::Element* {
		return &udmData.GetRootElement();
	}));
	cdData.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return udmData.GetRootElement()[key];
	}));
	cdData.def("ResolveReferences",static_cast<void(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) {
		udmData.ResolveReferences();
	}));
	modUdm[cdData];

	auto cdPropWrap = luabind::class_<::udm::PropertyWrapper>("PropertyWrapper");
	cdPropWrap.def(luabind::constructor<>());
	cdPropWrap.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::PropertyWrapper,::udm::PropertyWrapper&>(cdPropWrap);
	modUdm[cdPropWrap];

	auto cdLinkedPropWrap = luabind::class_<::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("LinkedPropertyWrapper");
	cdLinkedPropWrap.def(luabind::tostring(luabind::self));
	cdLinkedPropWrap.def(luabind::constructor<>());
	cdLinkedPropWrap.def("GetAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> ::udm::AssetData {
		return ::udm::AssetData{prop};
	}));
	cdLinkedPropWrap.def("GetPath",static_cast<std::string(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> std::string {
		return prop.GetPath();
	}));
	cdLinkedPropWrap.def("ClaimOwnership",static_cast<::udm::PProperty(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> ::udm::PProperty {
		return prop.ClaimOwnership();
	}));
	modUdm[cdLinkedPropWrap];

	auto cdAssetData = luabind::class_<::udm::AssetData,::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("AssetData");
	cdAssetData.def(luabind::tostring(luabind::self));
	cdAssetData.def("GetAssetType",&::udm::AssetData::GetAssetType);
	cdAssetData.def("GetAssetVersion",&::udm::AssetData::GetAssetVersion);
	cdAssetData.def("SetAssetType",&::udm::AssetData::SetAssetType);
	cdAssetData.def("SetAssetVersion",&::udm::AssetData::SetAssetVersion);
	cdAssetData.def("GetData",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::AssetData&)>([](lua_State *l,::udm::AssetData &a) -> ::udm::LinkedPropertyWrapper {return a.GetData();}));
	cdAssetData.def("SetData",static_cast<void(*)(lua_State*,::udm::AssetData&,::udm::PProperty&)>([](lua_State *l,::udm::AssetData &a,::udm::PProperty &prop) {
		a.GetData() = prop;
	}));
	modUdm[cdAssetData];

	auto cdArray = luabind::class_<::udm::Array>("Array");
	cdArray.def(luabind::tostring(luabind::self));
	modUdm[cdArray];

	auto cdProp = luabind::class_<::udm::Property>("Property");
	cdProp.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::Property,::udm::PropertyWrapper>(cdProp);
	cdProp.def("GetType",static_cast<::udm::Type(*)(lua_State*,::udm::Property&)>([](lua_State *l,::udm::Property &prop) -> ::udm::Type {
		return prop.type;
	}));
	cdProp.def("ToAscii",static_cast<std::string(*)(
		lua_State*,::udm::Property&,const std::string&,const std::string&)>([](lua_State *l,::udm::Property &prop,const std::string &propName,const std::string &prefix
	) -> std::string {
		std::stringstream ss;
		prop.ToAscii(::udm::AsciiSaveFlags::None,ss,propName,prefix);
		return ss.str();
	}));
	cdProp.def("ToAscii",static_cast<std::string(*)(
		lua_State*,::udm::Property&,const std::string&)>([](lua_State *l,::udm::Property &prop,const std::string &propName
	) -> std::string {
		std::stringstream ss;
		prop.ToAscii(::udm::AsciiSaveFlags::None,ss,propName);
		return ss.str();
	}));
	cdProp.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::Property&)>([](lua_State *l,::udm::Property &prop) -> ::udm::LinkedPropertyWrapper {
		return ::udm::LinkedPropertyWrapper{prop};
	}));
	modUdm[cdProp];
	
	// TODO: Register "Get" method as subscript operator for the Lua classes
	// Attempts below don't work
	/*auto *reg = luabind::detail::class_registry::get_registry(lua.GetState());
	auto *cls = reg->find_class(typeid(::udm::Property));
	lua_rawgeti(lua.GetState(), LUA_REGISTRYINDEX, cls->metatable_ref());
	auto o = luabind::object{luabind::from_stack(lua.GetState(),-1)};
	Lua::Pop(lua.GetState(),1);
	o["__index"] = luabind::make_function(lua.GetState(),static_cast<void(*)()>([]() {
		std::cout<<"__index: "<<std::endl;
	}));*/
	//luabind::detail::add_overload(o,"__index",luabind::make_function(lua.GetState(),static_cast<void(*)()>([]() {
	//	std::cout<<"__index: "<<std::endl;
	//})));

	/*auto metaTable = luabind::getmetatable(cdProp);
	metaTable["__index"] = luabind::make_function(lua.GetState(),static_cast<void(*)()>([]() {
		std::cout<<"__index: "<<std::endl;
	}));
	metaTable["__newindex"] = luabind::make_function(lua.GetState(),static_cast<void(*)()>([]() {
		std::cout<<"__newindex: "<<std::endl;
	}));*/
	/*auto r = Lua::RunString(lua.GetState(),R""""(
		local function addSubscriptOperator(class)
			local meta = debug.getmetatable(class)
			local oldIndex = meta.__index
			function meta:__index(key)
				local prop = self:Get(key)
				if(prop:IsValid()) then return prop end
				return oldIndex(self,key)
			end
		end
		addSubscriptOperator(udm.Element)
		addSubscriptOperator(udm.PropertyWrapper)
		addSubscriptOperator(udm.Property)
	)"""","internal");
	if(r != Lua::StatusCode::Ok)
		Lua::HandleLuaError(lua.GetState());*/
}
#pragma optimize("",on)
