/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/util.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/types/udm.hpp"
#include "pragma/util/util_game.hpp"
#include <sharedutils/util_path.hpp>
#include <luabind/iterator_policy.hpp>
#include <luainterface.hpp>
#include <udm.hpp>
#include <datasystem_vector.h>
#include <datasystem_color.h>
#include <sharedutils/util_file.h>
#include <fsys/ifile.hpp>

extern DLLNETWORK Engine *engine;


template<class TPropertyWrapper,class TClass,typename T>
	void prop_set_basic_type_indexed(lua_State *l,TClass &p,int32_t idx,const T &v)
{
	if(idx < 0)
		return;
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
		::udm::visit<true,false,false>(type,[l,idx,&p](auto tag){
			using T = decltype(tag)::type;
			if constexpr(std::is_same_v<T,::udm::Half>)
				p = ::udm::Half{static_cast<float>(Lua::CheckNumber(l,idx))};
			else
				p = Lua::udm::cast_object<T>(luabind::object{luabind::from_stack(l,idx)});
		});
	}
	if(::udm::is_generic_type(type))
	{
		::udm::visit<false,true,false>(type,[l,idx,&p](auto tag) {
			if constexpr(std::is_same_v<decltype(tag)::type,udm::Nil>)
				return;
			else
				p = static_cast<decltype(tag)::type&>(Lua::Check<decltype(tag)::type>(l,idx));
		});
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

static luabind::object get_array_values(lua_State *l,::udm::LinkedPropertyWrapperArg p,std::optional<::udm::Type> type)
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
	if(::udm::is_numeric_type(*type) || ::udm::is_generic_type(*type))
		udm::visit_ng(*type,vs);
	else if(::udm::is_non_trivial_type(*type))
	{
		udm::visit<false,false,true>(*type,[&vs,size,pArray,&t](auto tag) {
			using T = decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T,udm::String>)
				vs(tag);
			else if constexpr(std::is_same_v<T,udm::Element>)
			{
				for(auto i=decltype(size){0u};i<size;++i)
					t[i +1] = (*pArray)[i];
			}
		});
	}
	return t;
}

static luabind::object get_array_value(lua_State *l,::udm::LinkedPropertyWrapperArg p,int32_t idx,std::optional<::udm::Type> type)
{
	if(idx < 0)
		return Lua::nil;
	auto size = p.GetSize();
	auto *pArray = p.GetValuePtr<::udm::Array>();
	if(idx >= size || pArray == nullptr)
		return Lua::nil;

	if(!type.has_value())
		type = pArray->GetValueType();

	auto vs = [l,idx,pArray,size](auto tag) {
		using T = decltype(tag)::type;
		auto *p = static_cast<T*>(pArray->GetValues());
		return luabind::object{l,p[idx]};
	};
	if(::udm::is_numeric_type(*type) || ::udm::is_generic_type(*type))
		return udm::visit_ng(*type,vs);
	else if(::udm::is_non_trivial_type(*type))
	{
		return udm::visit<false,false,true>(*type,[l,&vs,size,pArray,idx](auto tag) -> luabind::object {
			using T = decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T,udm::String>)
				return vs(tag);
			else if constexpr(std::is_same_v<T,udm::Element>)
				return luabind::object{l,(*pArray)[idx]};
			return Lua::nil;
		});
	}
	return Lua::nil;
}

static luabind::object get_blob_array_values(lua_State *l,const ::udm::PropertyWrapper &p,::udm::Type type)
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
	if(::udm::is_ng_type(type))
		::udm::visit_ng(type,vs);
	return t;
}

static void set_blob_array_values(lua_State *l,const::udm::PropertyWrapper &p,const std::string &path,::udm::Type type,luabind::tableT<void> t,::udm::Type blobType=::udm::Type::BlobLz4)
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
	if(::udm::is_ng_type(type))
		::udm::visit_ng(type,vs);
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
	if(::udm::is_ng_type(type))
		return ::udm::visit_ng(type,vs);
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
static luabind::object get_property_value(lua_State *l,const ::udm::PropertyWrapper &val)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type type;
	auto *ptr = val.GetValuePtr(type);
	if(ptr == nullptr)
		return {};
	return get_property_value(l,type,ptr);
}
static luabind::object get_property_value(lua_State *l,const ::udm::PropertyWrapper &val,::udm::Type type)
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
	return udm::visit(type,vs);
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

template<typename T>
	static bool is_udm_type(luabind::object &val)
{
	return luabind::object_cast_nothrow<T*>(val,static_cast<T*>(nullptr)) != nullptr;
}
static udm::Type determine_udm_type(luabind::object &val)
{
	auto t = luabind::type(val);
	switch(t)
	{
	case LUA_TNIL:
		return udm::Type::Nil;
	case LUA_TBOOLEAN:
		return udm::Type::Boolean;
	case LUA_TSTRING:
		return udm::Type::String;
	case LUA_TNUMBER:
	{
		auto v = luabind::object_cast<double>(val);
		if(static_cast<int64_t>(v) == v)
		{
			if(v < 0.0)
				return udm::Type::UInt32;
			return udm::Type::Int32;
		}
		return udm::Type::Float;
	}
	case LUA_TUSERDATA:
		break;
	default:
		return udm::Type::Invalid;
	}

	if(is_udm_type<udm::Vector2>(val))
		return udm::Type::Vector2;
	if(is_udm_type<udm::Vector3>(val))
		return udm::Type::Vector3;
	if(is_udm_type<udm::Vector4>(val))
		return udm::Type::Vector4;
	if(is_udm_type<udm::Vector2i>(val))
		return udm::Type::Vector2i;
	if(is_udm_type<udm::Vector3i>(val))
		return udm::Type::Vector3i;
	if(is_udm_type<udm::Vector4i>(val))
		return udm::Type::Vector4i;
	if(is_udm_type<udm::Quaternion>(val))
		return udm::Type::Quaternion;
	if(is_udm_type<udm::EulerAngles>(val))
		return udm::Type::EulerAngles;
	if(is_udm_type<udm::Srgba>(val))
		return udm::Type::Srgba;
	if(is_udm_type<udm::HdrColor>(val))
		return udm::Type::HdrColor;
	if(is_udm_type<udm::Transform>(val))
		return udm::Type::Transform;
	if(is_udm_type<udm::ScaledTransform>(val))
		return udm::Type::ScaledTransform;
	if(is_udm_type<udm::Mat4>(val))
		return udm::Type::Mat4;
	if(is_udm_type<udm::Mat3x4>(val))
		return udm::Type::Mat3x4;
	if(is_udm_type<udm::String>(val))
		return udm::Type::String;
	if(is_udm_type<udm::Utf8String>(val))
		return udm::Type::Utf8String;
	if(is_udm_type<udm::Blob>(val))
		return udm::Type::Blob;
	if(is_udm_type<udm::BlobLz4>(val))
		return udm::Type::BlobLz4;
	if(is_udm_type<udm::Element>(val))
		return udm::Type::Element;
	if(is_udm_type<udm::Array>(val))
		return udm::Type::Array;
	if(is_udm_type<udm::ArrayLz4>(val))
		return udm::Type::ArrayLz4;
	if(is_udm_type<udm::Reference>(val))
		return udm::Type::Reference;
	if(is_udm_type<udm::Struct>(val))
		return udm::Type::Struct;
	return udm::Type::Invalid;
}

void Lua::udm::table_to_udm(const Lua::tb<void> &t,::udm::LinkedPropertyWrapper &udm)
{
	for(luabind::iterator it{t},end;it!=end;++it)
	{
		luabind::object val = *it;
		std::string key = luabind::object_cast<std::string>(luabind::object{it.key()});
		if(luabind::type(val) == LUA_TTABLE)
		{
			auto *l = val.interpreter();
			auto len = Lua::GetObjectLength(l,val);
			uint32_t actualLen = 0;
			auto isArrayType = true;
			std::pair<int,luabind::detail::class_rep*> typeInfo {-1,nullptr};
			for(luabind::iterator it{val},end;it!=end;++it)
			{
				auto t = luabind::type(*it);
				if(t != LUA_TUSERDATA)
				{
					if(typeInfo.second != nullptr)
					{
						isArrayType = false;
						break;
					}
					if(typeInfo.first == -1)
						typeInfo.first = t;
					else if(t != typeInfo.first)
					{
						isArrayType = false;
						break;
					}
					++actualLen;
					continue;
				}
				else
				{
					auto *crep = Lua::get_crep(val);
					if(!crep || typeInfo.first != -1)
					{
						isArrayType = false;
						break;
					}
					if(typeInfo.second == nullptr)
						typeInfo.second = crep;
					else if(crep != typeInfo.second)
					{
						isArrayType = false;
						break;
					}
				}
				++actualLen;
			}
			if(actualLen > 0 && isArrayType)
			{
				luabind::object first {val[1]};
				auto type = determine_udm_type(first);
				if(type != ::udm::Type::Invalid)
				{
					auto a = udm.AddArray(key,actualLen,type);
					::udm::visit(type,[actualLen,&a,&val](auto tag) {
						using T = decltype(tag)::type;
						constexpr auto type = ::udm::type_to_enum<T>();
						if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type))
						{
							for(auto i=decltype(actualLen){0u};i<actualLen;++i)
								a[i] = Lua::udm::cast_object<T>(luabind::object{val[i +1]});
						}
					});
				}
				continue;
			}
			auto udmSub = udm[key];
			table_to_udm(val,udmSub);
		}
		else
		{
			auto type = determine_udm_type(val);
			if(type != ::udm::Type::Invalid)
			{
				::udm::visit(type,[&udm,&key,&val](auto tag) {
					using T = decltype(tag)::type;
					constexpr auto type = ::udm::type_to_enum<T>();
					if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type))
						udm[key] = Lua::udm::cast_object<T>(val);
				});
			}
		}
	}
}
luabind::object Lua::udm::udm_to_value(lua_State *l,::udm::LinkedPropertyWrapperArg udm)
{
	auto type = udm.GetType();
	if(::udm::is_array_type(type))
	{
		auto t = luabind::newtable(l);
		for(uint32_t idx = 1;auto &val : udm)
			t[idx++] = udm_to_value(l,val);
		return t;
	}
	else if(type == ::udm::Type::Element)
	{
		auto t = luabind::newtable(l);
		for(auto &pair : const_cast<::udm::LinkedPropertyWrapper&>(udm).ElIt())
			t[pair.key] = udm_to_value(l,pair.property);
		return t;
	}
	return ::udm::visit(type,[l,&udm](auto tag) {
		using T = decltype(tag)::type;
		constexpr auto type = ::udm::type_to_enum<T>();
		if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type))
			return luabind::object{l,udm.GetValue<T>()};
		else
			return luabind::object{};
	});
}

static void data_block_to_udm(ds::Block &dataBlock,udm::LinkedPropertyWrapperArg udm)
{
	std::function<void(udm::LinkedPropertyWrapperArg,ds::Block&)> dataBlockToUdm = nullptr;
	dataBlockToUdm = [&dataBlockToUdm,&udm](udm::LinkedPropertyWrapperArg prop,ds::Block &block) {
		const_cast<udm::LinkedPropertyWrapper&>(prop).InitializeProperty();

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

static luabind::object get_children(lua_State *l,const ::udm::PropertyWrapper &p)
{
	auto t = luabind::newtable(l);
	auto *el = p.GetValuePtr<::udm::Element>();
	if(el == nullptr)
		return t;
	auto *linked = const_cast<::udm::PropertyWrapper&>(p).GetLinked();
	if(linked)
	{
		for(auto pair : linked->ElIt())
			t[std::string{pair.key}] = (*linked)[pair.key];
	}
	else
	{
		for(auto it=p.begin_el();it!=p.end_el();++it)
		{
			auto &pair = *it;
			t[std::string{pair.key}] = p[pair.key];
		}
	}
	return t;
}

template<typename T>
	static void set_array_values(udm::Array &a,::udm::Type type,luabind::tableT<void> t,size_t size,::udm::ArrayType arrayType)
{
	a.Resize(size);
	auto *pVal = static_cast<T*>(a.GetValues());
	for(auto i=decltype(size){0u};i<size;++i)
	{
		*pVal = Lua::udm::cast_object_nothrow<T>(luabind::object{t[i +1]});
		++pVal;
	}
}
template<typename T>
	static void set_array_values(udm::PropertyWrapper &p,const std::string &name,::udm::Type type,luabind::tableT<void> t,size_t size,::udm::ArrayType arrayType)
{
	auto a = p.AddArray(name,size,type,arrayType);
	set_array_values<T>(a.GetValue<udm::Array>(),type,t,size,arrayType);
}
template<typename T>
	concept is_assignable_type = !std::is_same_v<T,::udm::Element> && !std::is_same_v<T,::udm::Utf8String> && !std::is_same_v<T,::udm::Array> && !std::is_same_v<T,::udm::ArrayLz4>;
bool Lua::udm::set_array_value(lua_State *l,::udm::Array &a,int32_t idx,const luabind::object &o)
{
	if(idx < 0 || idx >= a.GetSize())
		return false;
	auto arrayType = a.GetArrayType();
	::udm::visit(a.GetValueType(),[&a,idx,&o,arrayType](auto tag) {
		using T = decltype(tag)::type;
		if constexpr(is_assignable_type<T>)
		{
			if(arrayType == ::udm::ArrayType::Raw)
			{
				if constexpr(::udm::Array::IsValueTypeSupported(::udm::type_to_enum<T>()))
				{
					auto *pVal = static_cast<T*>(a.GetValues());
					pVal[idx] = cast_object<T>(o);
				}
			}
			else if constexpr(::udm::ArrayLz4::IsValueTypeSupported(::udm::type_to_enum<T>()))
			{
				auto *pVal = static_cast<T*>(a.GetValues());
				pVal[idx] = cast_object<T>(o);
			}
		}
	});
	return true;
}
static void set_array_values(lua_State *l,::udm::Array &a,::udm::Type type,luabind::tableT<void> t,uint32_t tIdx,::udm::ArrayType arrayType=::udm::ArrayType::Raw)
{
	auto size = Lua::GetObjectLength(l,tIdx);
	auto vs = [&t,&a,type,tIdx,arrayType,size](auto tag) {
		using T = decltype(tag)::type;
		if constexpr(is_assignable_type<T>)
		{
			if(arrayType == ::udm::ArrayType::Raw)
			{
				if constexpr(::udm::Array::IsValueTypeSupported(::udm::type_to_enum<T>()))
					set_array_values<T>(a,type,t,size,arrayType);
			}
			else if constexpr(::udm::ArrayLz4::IsValueTypeSupported(::udm::type_to_enum<T>()))
				set_array_values<T>(a,type,t,size,arrayType);
		}
	};
	udm::visit(type,vs);
}
void Lua::udm::set_array_values(lua_State *l,::udm::Array &a,luabind::tableT<void> t,uint32_t tIdx)
{
	::set_array_values(l,a,a.GetValueType(),t,tIdx,a.GetArrayType());
}
static void set_array_values(lua_State *l,udm::PropertyWrapper &p,const std::string &name,::udm::Type type,luabind::tableT<void> t,::udm::ArrayType arrayType=::udm::ArrayType::Raw)
{
	auto size = Lua::GetObjectLength(l,4);
	auto a = p.AddArray(name,size,type,arrayType);
	set_array_values(l,a.GetValue<udm::Array>(),type,t,4,arrayType);
}

static ::udm::LinkedPropertyWrapper x_get(lua_State *l,::udm::Element &p,const std::string &key)
{
	return static_cast<::udm::PropertyWrapper&>(p)[key];
}
static ::udm::LinkedPropertyWrapper x_get(lua_State *l,::udm::PropertyWrapper &p,const std::string &key)
{
	return static_cast<::udm::PropertyWrapper&>(p)[key];
}
static ::udm::LinkedPropertyWrapper x_get(lua_State *l,::udm::Property &p,const std::string &key)
{
	return static_cast<::udm::PropertyWrapper>(p)[key];
}

static ::udm::LinkedPropertyWrapper i_get(lua_State *l,::udm::Element &p,int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<::udm::PropertyWrapper&>(p)[idx];
}
static ::udm::LinkedPropertyWrapper i_get(lua_State *l,::udm::PropertyWrapper &p,int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<::udm::PropertyWrapper&>(p)[idx];
}
static ::udm::LinkedPropertyWrapper i_get(lua_State *l,::udm::Property &p,int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<::udm::PropertyWrapper>(p)[idx];
}

template<class T,class TPropertyWrapper,class TClassDef>
	void register_property_methods(TClassDef &classDef)
{
	classDef.def("Add",+[](lua_State *l,T &p,const std::string &path) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).Add(path);
	})
	.def("Add",+[](lua_State *l,T &p,const std::string &path,::udm::Type type) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).Add(path,type);
	})
	.def("AddAssetData",+[](lua_State *l,T &p,const std::string &path) -> ::udm::AssetData {
		return ::udm::AssetData{static_cast<TPropertyWrapper>(p).Add(path)};
	})
	.def("ToAssetData",+[](lua_State *l,T &prop) -> ::udm::AssetData {
		return ::udm::AssetData {static_cast<TPropertyWrapper>(prop)};
	})
	//.def("Get",+[](lua_State *l,T &p,const std::string &key) -> ::udm::LinkedPropertyWrapper {
	//	return static_cast<TPropertyWrapper>(p)[key];
	//})
	.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,const std::string&)>(&x_get))
	.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,T&,int32_t)>(&i_get))
	//.def("Get",+[](lua_State *l,T &p,uint32_t idx) -> ::udm::LinkedPropertyWrapper {
	//	return static_cast<TPropertyWrapper>(p)[idx];
	//})
	.def("GetArrayValuesFromBlob",+[](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		return get_blob_array_values(l,static_cast<TPropertyWrapper>(p),type);
	})
	.def("AddBlobFromArrayValues",+[](lua_State *l,T &p,const std::string &path,::udm::Type type,luabind::tableT<void> t,::udm::Type blobType) {
		return set_blob_array_values(l,static_cast<TPropertyWrapper>(p),path,type,t,blobType);
	})
	.def("AddBlobFromArrayValues",+[](lua_State *l,T &p,const std::string &path,::udm::Type type,luabind::tableT<void> t) {
		return set_blob_array_values(l,static_cast<TPropertyWrapper>(p),path,type,t);
	})
	.def("AddValueRange",+[](lua_State *l,T &p,uint32_t startIndex,uint32_t count) {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a)
			return;
		::udm::Array::Range r0 {0 /* src */,0 /* dst */,startIndex};
		::udm::Array::Range r1 {startIndex /* src */,startIndex +count /* dst */,a->GetSize() -startIndex};
		a->Resize(a->GetSize() +count,r0,r1,false);
	})
	.def("RemoveValueRange",+[](lua_State *l,T &p,uint32_t startIndex,uint32_t count) {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a)
			return;
		::udm::Array::Range r0 {0 /* src */,0 /* dst */,startIndex};
		::udm::Array::Range r1 {startIndex +count /* src */,startIndex /* dst */,a->GetSize() -(startIndex +count)};
		a->Resize(a->GetSize() -count,r0,r1,false);
	})
	.def("GetBlobData",+[](lua_State *l,T &p) {
		auto type = ::udm::Type::Nil;
		auto vs = [l,&p,&type](auto tag) {
			using T = decltype(tag)::type;
			return static_cast<TPropertyWrapper>(p).GetBlobData(type);
		};
		::udm::Blob blob;
		if(::udm::is_ng_type(type))
			blob = ::udm::visit_ng(type,vs);
		else if(::udm::is_non_trivial_type(type))
			return; // TODO
		DataStream ds {static_cast<uint32_t>(blob.data.size())};
		ds->Write(blob.data.data(),blob.data.size());
		ds->SetOffset(0);
		Lua::Push(l,ds);
		Lua::Push(l,type);
	})
	.def("GetSize",+[](lua_State *l,T &p) -> uint32_t {
		return static_cast<TPropertyWrapper>(p).GetSize();
	})
	.def("GetValueType",+[](lua_State *l,T &p) -> luabind::object {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a)
			return luabind::object{};
		return luabind::object{l,umath::to_integral(a->GetValueType())};
	})
	.def("SetValueType",+[](lua_State *l,T &p,::udm::Type valueType) {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a)
			return;
		a->SetValueType(valueType);
	})
	.def("GetArrayValues",+[](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,*lp,type);
		return get_array_values(l,::udm::LinkedPropertyWrapper{tp},type);
	})
	.def("GetArrayValues",+[](lua_State *l,T &p) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,(*lp),{});
		return get_array_values(l,tp,{});
	})
	.def("GetArrayValues",+[](lua_State *l,T &p,const std::string &name,::udm::Type type) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,(*lp)[name],type);
		return get_array_values(l,tp[name],type);
	})
	.def("GetArrayValues",+[](lua_State *l,T &p,const std::string &name) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_values(l,(*lp)[name],{});
		return get_array_values(l,tp[name],{});
	})
	.def("GetArrayValue",+[](lua_State *l,T &p,int32_t idx,::udm::Type type) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_value(l,*lp,idx,type);
		return get_array_value(l,::udm::LinkedPropertyWrapper{tp},idx,type);
	})
	.def("GetArrayValue",+[](lua_State *l,T &p,int32_t idx) -> luabind::object {
		TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		auto *lp = tp.GetLinked();
		if(lp)
			return get_array_value(l,(*lp),idx,{});
		return get_array_value(l,tp,idx,{});
	})
	.def("SetArrayValues",+[](lua_State *l,T &p,::udm::Type type,luabind::tableT<void> t) {
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp.GetValue<::udm::Array>(),type,t,3);
	})
	.def("SetArrayValues",+[](lua_State *l,T &p,::udm::Type type,luabind::tableT<void> t,::udm::Type arrayType) {
		if(arrayType != ::udm::Type::Array && arrayType != ::udm::Type::ArrayLz4)
			Lua::Error(l,"Invalid array type '" +std::string{::udm::enum_type_to_ascii(arrayType)} +"'!");
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp.GetValue<::udm::Array>(),type,t,3,(arrayType == ::udm::Type::ArrayLz4) ? ::udm::ArrayType::Compressed : ::udm::ArrayType::Raw);
	})
	.def("SetArrayValues",+[](lua_State *l,T &p,const std::string &name,::udm::Type type,luabind::tableT<void> t) {
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp,name,type,t);
	})
	.def("SetArrayValues",+[](lua_State *l,T &p,const std::string &name,::udm::Type type,luabind::tableT<void> t,::udm::Type arrayType) {
		if(arrayType != ::udm::Type::Array && arrayType != ::udm::Type::ArrayLz4)
			Lua::Error(l,"Invalid array type '" +std::string{::udm::enum_type_to_ascii(arrayType)} +"'!");
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		set_array_values(l,tmp,name,type,t,(arrayType == ::udm::Type::ArrayLz4) ? ::udm::ArrayType::Compressed : ::udm::ArrayType::Raw);
	})
	.def("GetChildren",+[](lua_State *l,T &p) -> luabind::object {
		return get_children(l,static_cast<TPropertyWrapper>(p));
	})
	.def("GetChildren",+[](lua_State *l,T &p,const std::string &key) -> luabind::object {
		return get_children(l,static_cast<TPropertyWrapper>(p)[key]);
	})
	.def("GetChildCount",+[](lua_State *l,T &p) -> uint32_t {
		return static_cast<TPropertyWrapper>(p).GetChildCount();
	})
	.def("GetChildCount",+[](lua_State *l,T &p,const std::string &key) -> uint32_t {
		return static_cast<TPropertyWrapper>(p)[key].GetChildCount();
	})
	.def("Resize",+[](lua_State *l,T &p,uint32_t size) {
		static_cast<TPropertyWrapper>(p).Resize(size);
	})
	.def("RemoveValue",+[](lua_State *l,T &p,int32_t idx) {
		auto *a = static_cast<TPropertyWrapper>(p).GetValuePtr<::udm::Array>();
		if(!a || idx < 0)
			return;
		a->RemoveValue(idx);
	})
	.def("RemoveValue",+[](lua_State *l,T &p,const std::string &key) {
		TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		auto *el = tmp.GetValuePtr<::udm::Element>();
		if(!el)
			return;
		auto it = el->children.find(key);
		if(it != el->children.end())
			el->children.erase(it);
	})
	.def("SetValue",+[](lua_State *l,T &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],type,o,4);
	})
	.def("SetValue",+[](lua_State *l,T &el,int32_t idx,::udm::Type type,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[idx],type,o,4);
	})
	.def("SetValue",+[](lua_State *l,T &el,const std::string &key,::udm::Type type) {
		if(type != ::udm::Type::Nil)
			Lua::Error(l,"Omitting value only allowed for nil type!");
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],type,luabind::object{},4,true);
	})
	.def("SetValue",+[](lua_State *l,T &el,const std::string &key,luabind::object o) {
		set_property_value(l,static_cast<TPropertyWrapper>(el)[key],o);
	})
	.def("HasValue",+[](lua_State *l,T &p,const std::string &key) -> bool {
		return static_cast<bool>(static_cast<TPropertyWrapper>(p)[key]);
	})
	.def("GetValue",+[](lua_State *l,T &p,const std::string &key) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p)[key]);
	})
	.def("GetValue",+[](lua_State *l,T &p,const std::string &key,::udm::Type type) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p)[key],type);
	})
	.def("GetValue",+[](lua_State *l,T &p) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p));
	})
	.def("GetValue",+[](lua_State *l,T &p,::udm::Type type) -> luabind::object {
		return get_property_value(l,static_cast<TPropertyWrapper>(p),type);
	})
	.def("GetType",+[](lua_State *l,T &prop) -> ::udm::Type {
		return static_cast<TPropertyWrapper>(prop).GetType();
	})
	.def("Merge",+[](lua_State *l,T &prop,::udm::PropertyWrapper &propOther) {
		static_cast<TPropertyWrapper>(prop).Merge(propOther);
	})
	.def("Merge",+[](lua_State *l,T &prop,::udm::PropertyWrapper &propOther,::udm::MergeFlags mergeFlags) {
		static_cast<TPropertyWrapper>(prop).Merge(propOther,mergeFlags);
	})
	.def("Copy",+[](lua_State *l,T &prop) -> std::shared_ptr<::udm::Property> {
		auto p = static_cast<TPropertyWrapper>(prop).prop;
		if(!p)
			return nullptr;
		return std::make_shared<::udm::Property>(*p);
	})
	.def("ToAscii",+[](lua_State *l,T &prop) -> luabind::object {
		auto *el = static_cast<TPropertyWrapper>(prop).GetValuePtr<::udm::Element>();
		if(!el)
			return {};
		std::stringstream ss;
		el->ToAscii(::udm::AsciiSaveFlags::DontCompressLz4Arrays,ss);
		return luabind::object{l,ss.str()};
	})
	.def("ToTable",+[](lua_State *l,T &v) -> Lua::tb<void> {
		auto prop = static_cast<TPropertyWrapper>(v);
		auto o = Lua::udm::udm_to_value(l,prop);
		if(luabind::type(o) == LUA_TTABLE)
			return o;
		return luabind::newtable(l);
	})
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
	.def("Set",+[](lua_State *l,T &p,int32_t idx,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		static_cast<TPropertyWrapper>(p)[idx] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	})
	.def("Set",+[](lua_State *l,T &p,const std::string &key,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		static_cast<TPropertyWrapper>(p)[key] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	})
	.def("AddArray",+[](lua_State *l,T &p,const std::string &name,uint32_t size) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,::udm::Type::Element);
	})
	.def("AddArray",+[](lua_State *l,T &p,const std::string &name,uint32_t size,::udm::Type type) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,type);
	})
	.def("AddArray",+[](lua_State *l,T &p,const std::string &name,uint32_t size,::udm::Type type,::udm::ArrayType arrayType) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).AddArray(name,size,type,arrayType);
	})
	.def("GetFromPath",+[](lua_State *l,T &p,const std::string &path) -> ::udm::LinkedPropertyWrapper {
		return static_cast<TPropertyWrapper>(p).GetFromPath(path);
	})
	.def("IsValid",+[](lua_State *l,T &prop) -> bool {
		return static_cast<bool>(static_cast<TPropertyWrapper>(prop));
	});
}

static Lua::var<Lua::mult<bool,std::string>,::udm::Data> create(lua_State *l,::udm::PProperty rootProp,std::optional<std::string> assetType,std::optional<uint32_t> assetVersion)
{
	try
	{
		auto udmData = ::udm::Data::Create(assetType.has_value() ? *assetType : "",assetVersion.has_value() ? *assetVersion : 1);
		if(rootProp)
			udmData->GetAssetData().GetData() = rootProp;
		return luabind::object{l,udmData};
	}
	catch(const ::udm::Exception &e)
	{
		return Lua::mult<bool,std::string>{l,false,e.what()};
	}
}

static udm::Type determine_lua_object_udm_type(const luabind::object &o)
{
	auto type = static_cast<Lua::Type>(luabind::type(o));
	if(type == Lua::Type::Bool)
		return udm::Type::Boolean;
	if(type == Lua::Type::Number)
		return udm::Type::Float;
	if(type == Lua::Type::String)
		return udm::Type::String;

	auto isType = [&o]<typename T>() -> bool {
		return luabind::object_cast_nothrow<T*>(o,static_cast<T*>(nullptr)) != nullptr;
	};
	if(isType.template operator()<udm::Vector2>())
		return udm::Type::Vector2;
	if(isType.template operator()<udm::Vector3>())
		return udm::Type::Vector3;
	if(isType.template operator()<udm::Vector4>())
		return udm::Type::Vector4;
	if(isType.template operator()<udm::Quaternion>())
		return udm::Type::Quaternion;
	if(isType.template operator()<udm::EulerAngles>())
		return udm::Type::EulerAngles;
	if(isType.template operator()<udm::Srgba>())
		return udm::Type::Srgba;
	if(isType.template operator()<udm::HdrColor>())
		return udm::Type::HdrColor;
	if(isType.template operator()<udm::Transform>())
		return udm::Type::Transform;
	if(isType.template operator()<udm::ScaledTransform>())
		return udm::Type::ScaledTransform;
	if(isType.template operator()<udm::Mat4>())
		return udm::Type::Mat4;
	if(isType.template operator()<udm::Mat3x4>())
		return udm::Type::Mat3x4;
	if(isType.template operator()<udm::Vector2i>())
		return udm::Type::Vector2i;
	if(isType.template operator()<udm::Vector3i>())
		return udm::Type::Vector3i;
	if(isType.template operator()<udm::Vector4i>())
		return udm::Type::Vector4i;
	return udm::Type::Invalid;
}

template<typename T>
	static T &get_lua_object_udm_value(const luabind::object &o) requires(!std::is_arithmetic_v<T>)
{
	return *luabind::object_cast<T*>(o);
}

template<typename T>
	static T get_lua_object_udm_value(const luabind::object &o) requires(std::is_arithmetic_v<T>)
{
	return Lua::udm::cast_object<T>(o);
}

template<typename T>
	using lua_udm_underlying_numeric_type = 
		std::conditional_t<std::is_same_v<T,bool>,uint8_t,
		udm::underlying_numeric_type<T>
	>;

template<typename T>
	static lua_udm_underlying_numeric_type<T> get_numeric_component(const T &value,int32_t idx)
{

	if constexpr(udm::is_arithmetic<T>)
		return value;
	else if constexpr(udm::is_vector_type<T> || std::is_same_v<T,udm::EulerAngles> || std::is_same_v<T,udm::Srgba> || std::is_same_v<T,udm::HdrColor>)
		return value[idx];
	else if constexpr(std::is_same_v<T,udm::Quaternion>)
	{
		// Quaternion memory order is xyzw, but we want wxyz
		if(idx == 0)
			return value[3];
		return value[idx -1];
	}
	else
	{
		static_assert(std::is_same_v<lua_udm_underlying_numeric_type<T>,float>);
		return *(reinterpret_cast<const float*>(&value) +idx);
	}
}

static Lua::type<uint32_t> get_numeric_component(lua_State *l,const luabind::object &value,int32_t idx,udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : determine_lua_object_udm_type(value);
	return ::udm::visit_ng(type,[l,&value,idx](auto tag){
		using T = decltype(tag)::type;
		using BaseType = lua_udm_underlying_numeric_type<T>;
		if constexpr(std::is_same_v<lua_udm_underlying_numeric_type<T>,void>)
			return Lua::nil;
		else
			return luabind::object{l,get_numeric_component<T>(get_lua_object_udm_value<T>(value),idx)};
	});
}

static Lua::udm_ng lerp_value(lua_State *l,const luabind::object &value0,const luabind::object &value1,float t,udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : determine_lua_object_udm_type(value0);
	return ::udm::visit_ng(type,[l,&value0,&value1,t,type](auto tag){
		using T = decltype(tag)::type;
		T valuer;
		Lua::udm::lerp_value<T>(get_lua_object_udm_value<T>(value0),get_lua_object_udm_value<T>(value1),t,valuer,type);
		return luabind::object{l,valuer};
	});
}

void Lua::udm::register_library(Lua::Interface &lua)
{
	auto modUdm = luabind::module(lua.GetState(),"udm");
	modUdm[
		luabind::def("load",+[](lua_State *l,const std::string &fileName) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			std::string err;
			auto udmData = ::util::load_udm_asset(fileName,&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("load",+[](lua_State *l,LFile &file) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			if(!file.GetHandle())
				return Lua::mult<bool,std::string>{l,false,"Invalid file handle!"};
			std::string err;
			auto udmData = ::util::load_udm_asset(std::make_unique<fsys::File>(file.GetHandle()),&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("open",+[](lua_State *l,const std::string &fileName) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			try
			{
				auto udmData = ::udm::Data::Open(fileName);
				return luabind::object{l,udmData};
			}
			catch(const ::udm::Exception &e)
			{
				return Lua::mult<bool,std::string>{l,false,e.what()};
			}
		}),
		luabind::def("open",+[](lua_State *l,LFile &file) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			try
			{
				auto udmData = ::udm::Data::Open(file.GetHandle());
				return luabind::object{l,udmData};
			}
			catch(const ::udm::Exception &e)
			{
				return Lua::mult<bool,std::string>{l,false,e.what()};
			}
		}),
		luabind::def("create",&create),
		luabind::def("create",+[](lua_State *l,::udm::PProperty rootProp,std::optional<std::string> assetType) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			return create(l,rootProp,assetType,{});
		}),
		luabind::def("create",+[](lua_State *l,::udm::PProperty rootProp) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			return create(l,rootProp,{},{});
		}),
		luabind::def("create",+[](lua_State *l) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,{},{});
		}),
		luabind::def("create",+[](lua_State *l,std::optional<std::string> assetType,std::optional<uint32_t> assetVersion) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,assetType,assetVersion);
		}),
		luabind::def("create",+[](lua_State *l,std::optional<std::string> assetType) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,assetType,{});
		}),
		luabind::def("create_property",static_cast<::udm::PProperty(*)(::udm::Type)>(&::udm::Property::Create)),
		luabind::def("create_element",+[](lua_State *l) -> ::udm::PProperty {
			return ::udm::Property::Create(::udm::Type::Element);
		}),
		luabind::def("compress_lz4",+[](lua_State *l,DataStream ds) -> DataStream {
			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto lz4Blob = ::udm::compress_lz4_blob(ds->GetData(),ds->GetInternalSize());
			ds->SetOffset(offset);

			auto t = luabind::newtable(l);
			t[1] = lz4Blob.uncompressedSize;
			DataStream dsCompressed {static_cast<uint32_t>(lz4Blob.compressedData.size())};
			dsCompressed->Write(lz4Blob.compressedData.data(),lz4Blob.compressedData.size());
			t[2] = dsCompressed;
			return dsCompressed;
		}),
		luabind::def("decompress_lz4",+[](Lua::tb<void> t) -> DataStream {
			uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1],uint64_t{});
			DataStream ds = luabind::object_cast_nothrow<DataStream>(t[2],DataStream{});

			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto blob = ::udm::decompress_lz4_blob(ds->GetData(),ds->GetInternalSize(),uncompressedSize);
			ds->SetOffset(offset);

			DataStream dsDecompressed {static_cast<uint32_t>(blob.data.size())};
			dsDecompressed->Write(blob.data.data(),blob.data.size());
			return dsDecompressed;
		}),
		luabind::def("data_block_to_udm",&data_block_to_udm),
		luabind::def("data_file_to_udm",+[](const std::string &fileName) -> bool {
			std::string rpath;
			if(FileManager::FindAbsolutePath(fileName,rpath) == false)
				return false;
			auto f = FileManager::OpenFile(fileName.c_str(),"r");
			if(f == nullptr)
				return false;
			fsys::File fp {f};
			auto root = ds::System::ReadData(fp);
			if(root == nullptr)
				return false;
			ufile::remove_extension_from_filename(rpath);
			rpath += ".udm";
			auto p = util::Path::CreateFile(rpath);
			p.MakeRelative(util::get_program_path());
			rpath = p.GetString();
			auto fout = FileManager::OpenFile<VFilePtrReal>(rpath.c_str(),"w");
			if(fout == nullptr)
				return false;
			auto udmData = ::udm::Data::Create();
			data_block_to_udm(*root,udmData->GetAssetData().GetData());
			return udmData->SaveAscii(fout);
		}),
		luabind::def("debug_test",&::udm::Data::DebugTest),
		luabind::def("define_struct",+[](Lua::tb<void> t) -> ::udm::StructDescription {
			::udm::StructDescription structDesc {};
			auto n = Lua::GetObjectLength(t.interpreter(),t);
			structDesc.types.resize(n);
			structDesc.names.resize(n);
			for(auto i=decltype(n){0u};i<n;++i)
			{
				auto o = t[i +1];
				structDesc.types[i] = luabind::object_cast<::udm::Type>(o["type"]);
				structDesc.names[i] = luabind::object_cast<std::string>(o["name"]);
			}
			return structDesc;
		}),
		luabind::def("enum_type_to_ascii",&::udm::enum_type_to_ascii),
		luabind::def("ascii_type_to_enum",&::udm::ascii_type_to_enum),
		luabind::def("to_json",+[](::udm::LinkedPropertyWrapper &udm) -> std::string {
			std::stringstream ss;
			::udm::to_json(udm,ss);
			return ss.str();
		}),
		luabind::def("is_numeric_type",&::udm::is_numeric_type),
		luabind::def("is_integral_type",+[](::udm::Type type) -> bool {
			switch(type)
			{
			case ::udm::Type::Int8:
			case ::udm::Type::UInt8:
			case ::udm::Type::Int16:
			case ::udm::Type::UInt16:
			case ::udm::Type::Int32:
			case ::udm::Type::UInt32:
			case ::udm::Type::Int64:
			case ::udm::Type::UInt64:
			case ::udm::Type::Boolean:
				return true;
			}
			return false;
		}),
		luabind::def("is_floating_point_type",+[](::udm::Type type) -> bool {
			switch(type)
			{
			case ::udm::Type::Float:
			case ::udm::Type::Double:
			case ::udm::Type::Half:
				return true;
			}
			return false;
		}),
		luabind::def("is_generic_type",&::udm::is_generic_type),
		luabind::def("is_non_trivial_type",&::udm::is_non_trivial_type),
		luabind::def("get_numeric_component_count",&::udm::get_numeric_component_count),
		luabind::def("is_vector_type",+[](::udm::Type type) -> bool {
			return ::udm::visit<true,true,true>(type,[type](auto tag){
				using T = decltype(tag)::type;
				return ::udm::is_vector_type<T>;
			});
		}),
		luabind::def("is_matrix_type",+[](::udm::Type type) -> bool {
			return ::udm::visit<true,true,true>(type,[type](auto tag){
				using T = decltype(tag)::type;
				return ::udm::is_matrix_type<T>;
			});
		}),
		luabind::def("get_matrix_row_count",+[](::udm::Type type) -> uint32_t {
			switch(type)
			{
			case ::udm::Type::Mat4:
				return 4;
			case ::udm::Type::Mat3x4:
				return 3;
			}
			static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
			return 0;
		}),
		luabind::def("get_matrix_column_count",+[](::udm::Type type) -> uint32_t {
			switch(type)
			{
			case ::udm::Type::Mat4:
				return 4;
			case ::udm::Type::Mat3x4:
				return 4;
			}
			static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
			return 0;
		}),
		luabind::def("type_to_string",+[](::udm::Type type) -> std::string {
			return std::string{magic_enum::enum_name(type)};
		}),
		luabind::def("is_array_type",+[](::udm::Type type) -> bool {
			return ::udm::is_array_type(type);
		}),
		luabind::def("get_class_type",+[](lua_State *l,::udm::Type type) -> luabind::object {
			switch(type)
			{
			case ::udm::Type::Vector2:
				return luabind::globals(l)["math"]["Vector2"];
			case ::udm::Type::Vector2i:
				return luabind::globals(l)["math"]["Vector2i"];
			case ::udm::Type::Vector3:
				return luabind::globals(l)["math"]["Vector"];
			case ::udm::Type::Vector3i:
				return luabind::globals(l)["math"]["Vector3"];
			case ::udm::Type::Vector4:
				return luabind::globals(l)["math"]["Vector4"];
			case ::udm::Type::Vector4i:
				return luabind::globals(l)["math"]["Vector4i"];
			case ::udm::Type::Quaternion:
				return luabind::globals(l)["math"]["Quaternion"];
			case ::udm::Type::EulerAngles:
				return luabind::globals(l)["math"]["EulerAngles"];
			case ::udm::Type::Srgba:
				return luabind::globals(l)["udm"]["Srgba"];
			case ::udm::Type::HdrColor:
				return luabind::globals(l)["udm"]["HdrColor"];
			case ::udm::Type::Transform:
				return luabind::globals(l)["math"]["Transform"];
			case ::udm::Type::ScaledTransform:
				return luabind::globals(l)["math"]["ScaledTransform"];
			case ::udm::Type::Mat4:
				return luabind::globals(l)["math"]["Mat4"];
			case ::udm::Type::Mat3x4:
				return luabind::globals(l)["math"]["Mat3x4"];
			}
			static_assert(umath::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
			return Lua::nil;
		}),
		luabind::def("get_numeric_component",+[](lua_State *l,const luabind::object &value,int32_t idx) -> Lua::type<uint32_t> {
			return get_numeric_component(l,value,idx,::udm::Type::Invalid);
		}),
		luabind::def("get_numeric_component",static_cast<Lua::type<uint32_t>(*)(lua_State*,const luabind::object&,int32_t,::udm::Type)>(&get_numeric_component)),
		luabind::def("lerp",+[](lua_State *l,const luabind::object &value0,const luabind::object &value1,float t) -> Lua::udm_ng {
			return lerp_value(l,value0,value1,t,::udm::Type::Invalid);
		}),
		luabind::def("lerp",static_cast<Lua::udm_ng(*)(lua_State*,const luabind::object&,const luabind::object&,float,::udm::Type)>(&::lerp_value))
	];

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
        {"TYPE_INVALID",umath::to_integral(::udm::Type::Invalid)},

        {"ARRAY_TYPE_COMPRESSED",umath::to_integral(::udm::ArrayType::Compressed)},
        {"ARRAY_TYPE_RAW",umath::to_integral(::udm::ArrayType::Raw)},

		{"MERGE_FLAG_NONE",umath::to_integral(::udm::MergeFlags::None)},
		{"MERGE_FLAG_BIT_OVERWRITE_EXISTING",umath::to_integral(::udm::MergeFlags::OverwriteExisting)},
		{"MERGE_FLAG_BIT_DEEP_COPY",umath::to_integral(::udm::MergeFlags::DeepCopy)},
		
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
	cdRef.def("GetProperty",+[](lua_State *l,::udm::Reference &ref) -> ::udm::PropertyWrapper {
		return ref.property ? ::udm::PropertyWrapper{*ref.property} : ::udm::PropertyWrapper{};
	});
	modUdm[cdRef];
	
	auto cdStructDescription = luabind::class_<::udm::StructDescription>("StructDescription");
	cdStructDescription.def(luabind::constructor<>());
	cdStructDescription.def("GetTypes",+[](lua_State *l,::udm::StructDescription &udmData) -> luabind::object {
		return Lua::vector_to_table<::udm::Type>(l,udmData.types);
	});
	cdStructDescription.def("GetNames",+[](lua_State *l,::udm::StructDescription &udmData) -> luabind::object {
		return Lua::vector_to_table<std::string>(l,udmData.names);
	});
	cdStructDescription.def("GetMemberCount",+[](lua_State *l,::udm::StructDescription &udmData) -> uint32_t {
		return udmData.GetMemberCount();
	});
	modUdm[cdStructDescription];
	
	auto cdStruct = luabind::class_<::udm::Struct>("Struct");
	cdStruct.def(luabind::constructor<>());
	cdStruct.def(luabind::constructor<const ::udm::StructDescription&>());
	cdStruct.def("GetDescription",+[](lua_State *l,::udm::Struct &strct) -> ::udm::StructDescription* {
		return &strct.description;
	});
	cdStruct.def("GetValue",+[](lua_State *l,::udm::Struct &strct) -> luabind::object {
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
			t[i +1] = ::udm::visit<false,true,false>(strct.description.types[i],vs);
			ptr += ::udm::size_of(strct.description.types[i]);
		}
		return t;
	});
	modUdm[cdStruct];

	auto cdSrgba = luabind::class_<::udm::Srgba>("Srgba");
	cdSrgba.def(luabind::constructor<>());
	cdSrgba.def(luabind::tostring(luabind::self));
	cdSrgba.property("r",+[](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[0];
	},+[](lua_State *l,::udm::Srgba &srgba,uint8_t r) {
		srgba[0] = r;
	});
	cdSrgba.property("g",+[](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[1];
	},+[](lua_State *l,::udm::Srgba &srgba,uint8_t g) {
		srgba[1] = g;
	});
	cdSrgba.property("b",+[](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[2];
	},+[](lua_State *l,::udm::Srgba &srgba,uint8_t b) {
		srgba[2] = b;
	});
	cdSrgba.property("a",+[](lua_State *l,::udm::Srgba &srgba) -> uint8_t {
		return srgba[3];
	},+[](lua_State *l,::udm::Srgba &srgba,uint8_t a) {
		srgba[3] = a;
	});
	modUdm[cdSrgba];

	auto cdHdr = luabind::class_<::udm::HdrColor>("HdrColor");
	cdHdr.def(luabind::constructor<>());
	cdHdr.def(luabind::tostring(luabind::self));
	cdHdr.property("r",+[](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[0];
	},+[](lua_State *l,::udm::HdrColor &srgba,uint8_t r) {
		srgba[0] = r;
	});
	cdHdr.property("g",+[](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[1];
	},+[](lua_State *l,::udm::HdrColor &srgba,uint8_t g) {
		srgba[1] = g;
	});
	cdHdr.property("b",+[](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[2];
	},+[](lua_State *l,::udm::HdrColor &srgba,uint8_t b) {
		srgba[2] = b;
	});
	cdHdr.property("a",+[](lua_State *l,::udm::HdrColor &srgba) -> uint8_t {
		return srgba[3];
	},+[](lua_State *l,::udm::HdrColor &srgba,uint8_t a) {
		srgba[3] = a;
	});
	modUdm[cdHdr];

	auto cdData = luabind::class_<::udm::Data>("Data");
	cdData.def(luabind::tostring(luabind::self));
	cdData.def("Save",+[](lua_State *l,::udm::Data &udmData,const std::string &fileName) {
		auto fname = fileName;
		if(Lua::file::validate_write_operation(l,fname) == false)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,"Invalid write location!");
			return;
		}
		try
		{
			udmData.Save(fname);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	});
	cdData.def("SaveAscii",+[](lua_State *l,::udm::Data &udmData,const std::string &fileName) {
		auto fname = fileName;
		if(Lua::file::validate_write_operation(l,fname) == false)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,"Invalid write location!");
			return;
		}
		try
		{
			udmData.SaveAscii(fname);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	});
	cdData.def("SaveAscii",+[](lua_State *l,::udm::Data &udmData,const std::string &fileName,::udm::AsciiSaveFlags flags) {
		auto fname = fileName;
		if(Lua::file::validate_write_operation(l,fname) == false)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,"Invalid write location!");
			return;
		}
		try
		{
			udmData.SaveAscii(fname,flags);
			Lua::PushBool(l,true);
		}
		catch(const ::udm::Exception &e)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,e.what());
		}
	});
	cdData.def("Save",+[](lua_State *l,::udm::Data &udmData,LFile &f) {
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
	});
	cdData.def("SaveAscii",+[](lua_State *l,::udm::Data &udmData,LFile &f) {
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
	});
	cdData.def("SaveAscii",+[](lua_State *l,::udm::Data &udmData,LFile &f,::udm::AsciiSaveFlags flags) {
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
	});
	cdData.def("ToAscii",+[](lua_State *l,::udm::Data &udmData) -> std::string {
		std::stringstream ss;
		udmData.ToAscii(ss);
		return ss.str();
	});
	cdData.def("GetAssetData",+[](lua_State *l,::udm::Data &udmData) -> ::udm::AssetData {return udmData.GetAssetData();});
	cdData.def("LoadProperty",+[](lua_State *l,::udm::Data &udmData,const std::string &path) {
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
	});
	cdData.def("GetRootElement",+[](lua_State *l,::udm::Data &udmData) -> ::udm::Element* {
		return &udmData.GetRootElement();
	});
	cdData.def("Get",+[](lua_State *l,::udm::Data &udmData,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return udmData.GetRootElement()[key];
	});
	cdData.def("ResolveReferences",+[](lua_State *l,::udm::Data &udmData) {
		udmData.ResolveReferences();
	});
	modUdm[cdData];

	auto cdPropWrap = luabind::class_<::udm::PropertyWrapper>("PropertyWrapper");
	cdPropWrap.def(luabind::constructor<>());
	cdPropWrap.def(luabind::tostring(luabind::self));
	static std::optional<LuaUdmArrayIterator> g_it {}; // HACK: This is a workaround for a bug in luabind, which causes errors when compiled with gcc.
	cdPropWrap.def("It",+[](lua_State *l,::udm::PropertyWrapper &p) -> LuaUdmArrayIterator& {
		g_it = LuaUdmArrayIterator{p};
		return *g_it;
	},luabind::return_stl_iterator{});
	register_property_methods<::udm::PropertyWrapper,::udm::PropertyWrapper&>(cdPropWrap);
	modUdm[cdPropWrap];

	auto cdLinkedPropWrap = luabind::class_<::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("LinkedPropertyWrapper");
	cdLinkedPropWrap.def(luabind::tostring(luabind::self));
	cdLinkedPropWrap.def(luabind::constructor<>());
	cdLinkedPropWrap.def("GetAssetData",+[](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> ::udm::AssetData {
		return ::udm::AssetData{prop};
	});
	cdLinkedPropWrap.def("GetPath",+[](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> std::string {
		return prop.GetPath();
	});
	cdLinkedPropWrap.def("ClaimOwnership",+[](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> ::udm::PProperty {
		return prop.ClaimOwnership();
	});
	modUdm[cdLinkedPropWrap];

	auto cdAssetData = luabind::class_<::udm::AssetData,::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("AssetData");
	cdAssetData.def(luabind::tostring(luabind::self));
	cdAssetData.def("GetAssetType",&::udm::AssetData::GetAssetType);
	cdAssetData.def("GetAssetVersion",&::udm::AssetData::GetAssetVersion);
	cdAssetData.def("SetAssetType",&::udm::AssetData::SetAssetType);
	cdAssetData.def("SetAssetVersion",&::udm::AssetData::SetAssetVersion);
	cdAssetData.def("GetData",+[](lua_State *l,::udm::AssetData &a) -> ::udm::LinkedPropertyWrapper {return a.GetData();});
	cdAssetData.def("SetData",+[](lua_State *l,::udm::AssetData &a,::udm::PProperty &prop) {
		a.GetData() = prop;
	});
	modUdm[cdAssetData];

	auto cdArray = luabind::class_<::udm::Array>("Array");
	cdArray.def(luabind::tostring(luabind::self));
	cdArray.def("GetArrayType",&::udm::Array::GetArrayType);
	cdArray.def("ToTable",+[](lua_State *l,::udm::Array &a) -> Lua::tb<void> {
		auto t = luabind::newtable(l);
		for(uint32_t idx = 1;auto &val : a)
			t[idx++] = udm_to_value(l,val);
		return t;
	});
	modUdm[cdArray];

	auto cdProp = luabind::class_<::udm::Property>("Property");
	cdProp.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::Property,::udm::PropertyWrapper>(cdProp);
	cdProp.def("GetType",+[](lua_State *l,::udm::Property &prop) -> ::udm::Type {
		return prop.type;
	});
	cdProp.def("ToAscii",+[](lua_State *l,::udm::Property &prop,const std::string &propName,const std::string &prefix) -> std::string {
		std::stringstream ss;
		prop.ToAscii(::udm::AsciiSaveFlags::None,ss,propName,prefix);
		return ss.str();
	});
	cdProp.def("ToAscii",+[](lua_State *l,::udm::Property &prop,const std::string &propName) -> std::string {
		std::stringstream ss;
		prop.ToAscii(::udm::AsciiSaveFlags::None,ss,propName);
		return ss.str();
	});
	cdProp.def("Get",+[](lua_State *l,::udm::Property &prop) -> ::udm::LinkedPropertyWrapper {
		return ::udm::LinkedPropertyWrapper{prop};
	});
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
