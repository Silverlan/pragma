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
template<typename T>
	void element_set_basic_type(lua_State *l,::udm::Element &el,const std::string &key,const T &v)
{
	el.fromProperty[key] = v;
}

template<typename T>
	void prop_set_basic_type_indexed(lua_State *l,::udm::LinkedPropertyWrapper &p,uint32_t idx,const T &v)
{
	p[idx] = v;
}

template<typename T>
	void prop_set_basic_type(lua_State *l,::udm::LinkedPropertyWrapper &p,const std::string &key,const T &v)
{
	p[key] = v;
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

static void set_property_value(lua_State *l,::udm::LinkedPropertyWrapper p,::udm::Type type,luabind::object o)
{
	if(::udm::is_numeric_type(type))
	{
		auto tag = ::udm::get_numeric_tag(type);
		std::visit([l,&p](auto tag){p = static_cast<decltype(tag)::type>(Lua::CheckNumber(l,4));},tag);
	}
	if(::udm::is_generic_type(type))
	{
		auto tag = ::udm::get_generic_tag(type);
		std::visit([l,&p](auto tag) {
			if constexpr(std::is_same_v<decltype(tag)::type,udm::Nil>)
				return;
			else
				p = static_cast<decltype(tag)::type&>(Lua::Check<decltype(tag)::type>(l,4));
		},tag);
	}
	switch(type)
	{
	case ::udm::Type::String:
		p = static_cast<::udm::String>(Lua::CheckString(l,4));
		break;
	case ::udm::Type::Utf8String:
		p = data_stream_to_utf8(Lua::Check<DataStream>(l,4));
		break;
	case ::udm::Type::Blob:
		p = data_stream_to_blob(Lua::Check<DataStream>(l,4));
		break;
	case ::udm::Type::BlobLz4:
	{
		Lua::CheckTable(l,4);
		luabind::table<> t {luabind::from_stack(l,4)};
		uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1],uint64_t{});
		DataStream ds = luabind::object_cast_nothrow<DataStream>(t[2],DataStream{});
		p = data_stream_to_lz4_blob(ds,uncompressedSize);
		break;
	}
	case ::udm::Type::HdrColor:
	{
		Lua::CheckTable(l,4);
		luabind::table<> t {luabind::from_stack(l,4)};
		auto color = Lua::table_to_vector<uint16_t>(l,t,4);
		auto def = static_cast<uint16_t>(0);
		p = ::udm::HdrColor{(color.size() > 0) ? color[0] : def,(color.size() > 1) ? color[1] : def,(color.size() > 2) ? color[2] : def};
		break;
	}
	}
	static_assert(umath::to_integral(::udm::Type::Count) == 29,"Update this when types have been added or removed!");
}

static luabind::object get_array_values(lua_State *l,::udm::PropertyWrapper &p,::udm::Type type)
{
	auto t = luabind::newtable(l);
	auto size = p.GetSize();
	auto *pArray = p.GetValuePtr<::udm::Array>();
	if(size == 0 || pArray == nullptr)
		return t;
	auto vs = [&t,pArray,size](auto tag) {
		using T = decltype(tag)::type;
		auto *p = static_cast<T*>(pArray->values);
		for(auto i=decltype(size){0u};i<size;++i)
		{
			t[i +1] = *p;
			++p;
		}
	};
	if(::udm::is_numeric_type(type))
		std::visit(vs,::udm::get_numeric_tag(type));
	else if(::udm::is_generic_type(type))
		std::visit(vs,::udm::get_generic_tag(type));
	else if(::udm::is_non_trivial_type(type))
	{
		std::visit([&vs](auto tag) {
			using T = decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T,udm::String>)
				vs(tag);
		},::udm::get_non_trivial_tag(type));
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

static luabind::object get_property_value(lua_State *l,::udm::LinkedPropertyWrapper val)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type type;
	auto *ptr = val.GetValuePtr(type);
	if(ptr == nullptr)
		return {};
	auto vs = [l,ptr](auto tag) {
		if constexpr(std::is_same_v<decltype(tag)::type,udm::Nil>)
			return luabind::object{};
		else
			return luabind::object{l,*static_cast<decltype(tag)::type*>(ptr)};
	};
	if(::udm::is_numeric_type(type))
		return std::visit(vs,::udm::get_numeric_tag(type));
	if(::udm::is_generic_type(type))
		return std::visit(vs,::udm::get_generic_tag(type));
	switch(type)
	{
	case ::udm::Type::String:
		return luabind::object{l,*static_cast<udm::String*>(ptr)};
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
	case ::udm::Type::HdrColor:
	{
		auto &hdrCol = *static_cast<udm::HdrColor*>(ptr);
		auto t = luabind::newtable(l);
		for(auto i=decltype(hdrCol.size()){0u};i<hdrCol.size();++i)
			t[i +1] = hdrCol[i];
		return t;
	}
	}
	static_assert(umath::to_integral(::udm::Type::Count) == 29,"Update this when types have been added or removed!");
	return {};
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
			if(Lua::IsSet(l,1))
				assetType = Lua::CheckString(l,1);
			if(Lua::IsSet(l,2))
				assetVersion = Lua::CheckInt(l,2);

			try
			{
				auto udmData = ::udm::Data::Create(assetType,assetVersion);
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
			Lua::PushBool(l,udmData->SaveAscii(fout,false));
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
        {"TYPE_VECTOR3",umath::to_integral(::udm::Type::Vector3)},
        {"TYPE_VECTOR4",umath::to_integral(::udm::Type::Vector4)},
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
        {"TYPE_COUNT",umath::to_integral(::udm::Type::Count)}
	});
	static_assert(umath::to_integral(::udm::Type::Count) == 29,"Update this list when types have been added or removed!");

	auto cdEl = luabind::class_<::udm::Element>("Element");
	cdEl.def("Add",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::Element&,const std::string&)>([](lua_State *l,::udm::Element &el,const std::string &name) -> ::udm::PropertyWrapper {
		return el.Add(name);
	}));
	cdEl.def("AddArray",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::Element&,const std::string&,uint32_t)>([](lua_State *l,::udm::Element &el,const std::string &name,uint32_t size) -> ::udm::PropertyWrapper {
		return el.AddArray(name,size,::udm::Type::Element);
	}));
	cdEl.def("AddArray",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::Element&,const std::string&,uint32_t,::udm::Type)>([](lua_State *l,::udm::Element &el,const std::string &name,uint32_t size,::udm::Type type) -> ::udm::PropertyWrapper {
		return el.AddArray(name,size,type);
	}));
	cdEl.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::Element&,const std::string&)>([](lua_State *l,::udm::Element &el,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return el.fromProperty[key];
	}));
	cdEl.def("GetChildren",static_cast<luabind::object(*)(lua_State*,::udm::Element&)>([](lua_State *l,::udm::Element &el) -> luabind::object {
		auto t = luabind::newtable(l);
		for(auto &pair : el.children)
			t[pair.first] = ::udm::LinkedPropertyWrapper{*pair.second};
		return t;
	}));
	cdEl.def("SetValue",static_cast<void(*)(lua_State*,::udm::Element&,const std::string&,::udm::Type,luabind::object)>([](lua_State *l,::udm::Element &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,el.fromProperty[key],type,o);
	}));
	cdEl.def("GetValue",static_cast<luabind::object(*)(lua_State*,::udm::Element&,const std::string&)>([](lua_State *l,::udm::Element &el,const std::string &key) -> luabind::object {
		return get_property_value(l,el[key]);
	}));
	cdEl.def("Set",&element_set_basic_type<bool>);
	cdEl.def("Set",&element_set_basic_type<float>);
	cdEl.def("Set",&element_set_basic_type<Vector2>);
	cdEl.def("Set",&element_set_basic_type<Vector3>);
	cdEl.def("Set",&element_set_basic_type<Vector4>);
	cdEl.def("Set",&element_set_basic_type<Quat>);
	cdEl.def("Set",&element_set_basic_type<EulerAngles>);
	cdEl.def("Set",&element_set_basic_type<umath::Transform>);
	cdEl.def("Set",&element_set_basic_type<umath::ScaledTransform>);
	cdEl.def("Set",&element_set_basic_type<Mat4>);
	cdEl.def("Set",&element_set_basic_type<Mat3x4>);
	cdEl.def("Set",&element_set_basic_type<std::string>);
	cdEl.def("Set",static_cast<void(*)(lua_State*,::udm::Element&,const std::string&,DataStream&)>([](lua_State *l,::udm::Element &el,const std::string &key,DataStream &ds) {el.fromProperty[key] = data_stream_to_blob(ds);}));
	modUdm[cdEl];

	auto cdData = luabind::class_<::udm::Data>("Data");
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
	modUdm[cdData];

	auto cdPropWrap = luabind::class_<::udm::PropertyWrapper>("PropertyWrapper");
	cdPropWrap.def("It",static_cast<LuaUdmArrayIterator(*)(lua_State*,::udm::PropertyWrapper&)>([](lua_State *l,::udm::PropertyWrapper &p) -> LuaUdmArrayIterator {
		return LuaUdmArrayIterator{p};
	}),luabind::return_stl_iterator{});
	cdPropWrap.def("Add",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &path) -> ::udm::LinkedPropertyWrapper {
		return p.Add(path);
	}));
	cdPropWrap.def("Add",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &path,::udm::Type type) -> ::udm::LinkedPropertyWrapper {
		return p.Add(path,type);
	}));
	cdPropWrap.def("AddAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &path) -> ::udm::AssetData {
		return ::udm::AssetData{p.Add(path)};
	}));
	cdPropWrap.def("ToAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::PropertyWrapper&)>([](lua_State *l,::udm::PropertyWrapper &prop) -> ::udm::AssetData {
		return ::udm::AssetData {prop};
	}));
	cdPropWrap.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return p[key];
	}));
	cdPropWrap.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,uint32_t)>([](lua_State *l,::udm::PropertyWrapper &p,uint32_t idx) -> ::udm::LinkedPropertyWrapper {
		return p[idx];
	}));
	cdPropWrap.def("GetArrayValuesFromBlob",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&,::udm::Type)>([](lua_State *l,::udm::PropertyWrapper &p,::udm::Type type) -> luabind::object {
		return get_blob_array_values(l,p,type);
	}));
	cdPropWrap.def("AddBlobFromArrayValues",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type,luabind::table<>,::udm::Type)>(
		[](lua_State *l,::udm::PropertyWrapper &p,const std::string &path,::udm::Type type,luabind::table<> t,::udm::Type blobType) {
		return set_blob_array_values(l,p,path,type,t,blobType);
	}));
	cdPropWrap.def("AddBlobFromArrayValues",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type,luabind::table<>)>(
		[](lua_State *l,::udm::PropertyWrapper &p,const std::string &path,::udm::Type type,luabind::table<> t) {
		return set_blob_array_values(l,p,path,type,t);
	}));
	cdPropWrap.def("GetBlobData",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&)>([](lua_State *l,::udm::PropertyWrapper &p) {
		auto type = ::udm::Type::Nil;
		auto vs = [l,&p,&type](auto tag) {
			using T = decltype(tag)::type;
			return p.GetBlobData(type);
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
	}));
	cdPropWrap.def("GetSize",static_cast<uint32_t(*)(lua_State*,::udm::PropertyWrapper&)>([](lua_State *l,::udm::PropertyWrapper &p) -> uint32_t {
		return p.GetSize();
	}));
	cdPropWrap.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&,::udm::Type)>([](lua_State *l,::udm::PropertyWrapper &p,::udm::Type type) -> luabind::object {
		return get_array_values(l,p,type);
	}));
	cdPropWrap.def("GetArrayValues",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &name,::udm::Type type) -> luabind::object {
		return get_array_values(l,p[name],type);
	}));
	cdPropWrap.def("SetArrayValues",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type,luabind::table<>)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &name,::udm::Type type,luabind::table<> t) {
		auto size = Lua::GetObjectLength(l,3);
		auto vs = [&t,&name,&p,type,size](auto tag) {
			using T = decltype(tag)::type;
			auto &a = p.AddArray(name,size,type);
			auto *p = static_cast<T*>(a.GetValue<::udm::Array>().values);
			for(auto i=decltype(size){0u};i<size;++i)
			{
				*p = luabind::object_cast_nothrow<T>(t[i +1],T{});
				++p;
			}
		};
		if(::udm::is_numeric_type(type))
			std::visit(vs,::udm::get_numeric_tag(type));
		else if(::udm::is_generic_type(type))
			std::visit(vs,::udm::get_generic_tag(type));
	}));
	cdPropWrap.def("GetChildren",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&)>([](lua_State *l,::udm::PropertyWrapper &p) -> luabind::object {
		auto t = luabind::newtable(l);
		auto *el = p.GetValuePtr<::udm::Element>();
		if(el == nullptr)
			return t;
		for(auto &pair : el->children)
			t[pair.first] = ::udm::LinkedPropertyWrapper{*pair.second};
		return t;
	}));
	cdPropWrap.def("Resize",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,uint32_t)>([](lua_State *l,::udm::PropertyWrapper &p,uint32_t size) {
		p.Resize(size);
	}));
	cdPropWrap.def("SetValue",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type,luabind::object)>([](lua_State *l,::udm::PropertyWrapper &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,el[key],type,o);
	}));
	cdPropWrap.def("GetValue",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &key) -> luabind::object {
		return get_property_value(l,p[key]);
	}));
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<bool>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<float>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Vector2>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Vector3>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Vector4>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Quat>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<EulerAngles>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<umath::Transform>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<umath::ScaledTransform>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Mat4>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<Mat3x4>);
	cdPropWrap.def("Set",&prop_set_basic_type_indexed<std::string>);

	cdPropWrap.def("Set",&prop_set_basic_type<bool>);
	cdPropWrap.def("Set",&prop_set_basic_type<float>);
	cdPropWrap.def("Set",&prop_set_basic_type<Vector2>);
	cdPropWrap.def("Set",&prop_set_basic_type<Vector3>);
	cdPropWrap.def("Set",&prop_set_basic_type<Vector4>);
	cdPropWrap.def("Set",&prop_set_basic_type<Quat>);
	cdPropWrap.def("Set",&prop_set_basic_type<EulerAngles>);
	cdPropWrap.def("Set",&prop_set_basic_type<umath::Transform>);
	cdPropWrap.def("Set",&prop_set_basic_type<umath::ScaledTransform>);
	cdPropWrap.def("Set",&prop_set_basic_type<Mat4>);
	cdPropWrap.def("Set",&prop_set_basic_type<Mat3x4>);
	cdPropWrap.def("Set",&prop_set_basic_type<std::string>);
	cdPropWrap.def("Set",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,uint32_t,DataStream&)>([](lua_State *l,::udm::PropertyWrapper &p,uint32_t idx,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		p[idx] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	}));
	cdPropWrap.def("Set",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,DataStream&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &key,DataStream &ds) {
		auto offset = ds->GetOffset();
		ds->SetOffset(0);
		std::vector<uint8_t> data {};
		data.resize(ds->GetInternalSize());
		memcpy(data.data(),ds->GetData(),data.size());
		p[key] = ::udm::Blob {std::move(data)};
		ds->SetOffset(offset);
	}));
	cdPropWrap.def("AddArray",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,uint32_t)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &name,uint32_t size) -> ::udm::PropertyWrapper {
		return p.AddArray(name,size,::udm::Type::Element);
	}));
	cdPropWrap.def("AddArray",static_cast<::udm::PropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,uint32_t,::udm::Type)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &name,uint32_t size,::udm::Type type) -> ::udm::PropertyWrapper {
		return p.AddArray(name,size,type);
	}));
	modUdm[cdPropWrap];

	auto cdLinkedPropWrap = luabind::class_<::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("LinkedPropertyWrapper");
	cdLinkedPropWrap.def("GetAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::LinkedPropertyWrapper&)>([](lua_State *l,::udm::LinkedPropertyWrapper &prop) -> ::udm::AssetData {
		return ::udm::AssetData{prop};
	}));
	modUdm[cdLinkedPropWrap];

	auto cdAssetData = luabind::class_<::udm::AssetData,::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("AssetData");
	cdAssetData.def("GetAssetType",&::udm::AssetData::GetAssetType);
	cdAssetData.def("GetAssetVersion",&::udm::AssetData::GetAssetVersion);
	cdAssetData.def("SetAssetType",&::udm::AssetData::SetAssetType);
	cdAssetData.def("SetAssetVersion",&::udm::AssetData::SetAssetVersion);
	cdAssetData.def("GetData",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::AssetData&)>([](lua_State *l,::udm::AssetData &a) -> ::udm::LinkedPropertyWrapper {return a.GetData();}));
	modUdm[cdAssetData];

	auto cdArray = luabind::class_<::udm::Array>("Array");
	modUdm[cdArray];

	auto cdProp = luabind::class_<::udm::Property>("Property");
	cdProp.def("GetType",static_cast<::udm::Type(*)(lua_State*,::udm::Property&)>([](lua_State *l,::udm::Property &prop) -> ::udm::Type {
		return prop.type;
	}));
	cdProp.def("IsValid",static_cast<bool(*)(lua_State*,::udm::Property&)>([](lua_State *l,::udm::Property &prop) -> bool {
		return static_cast<bool>(prop);
	}));
	cdProp.def("GetValue",static_cast<luabind::object(*)(lua_State*,::udm::Property&)>([](lua_State *l,::udm::Property &prop) -> luabind::object {
		return get_property_value(l,::udm::LinkedPropertyWrapper{prop});
	}));
	cdProp.def("SetValue",static_cast<void(*)(lua_State*,::udm::Property&,::udm::Type,luabind::object)>([](lua_State *l,::udm::Property &prop,::udm::Type type,luabind::object o) {
		set_property_value(l,::udm::LinkedPropertyWrapper{prop},type,o);
	}));
	cdProp.def("ToAscii",static_cast<std::string(*)(lua_State*,::udm::Property&,const std::string&,const std::string&)>([](lua_State *l,::udm::Property &prop,const std::string &propName,const std::string &prefix) -> std::string {
		std::stringstream ss;
		prop.ToAscii(ss,propName,prefix);
		return ss.str();
	}));
	cdProp.def("ToAscii",static_cast<std::string(*)(lua_State*,::udm::Property&,const std::string&)>([](lua_State *l,::udm::Property &prop,const std::string &propName) -> std::string {
		std::stringstream ss;
		prop.ToAscii(ss,propName);
		return ss.str();
	}));
	modUdm[cdProp];
}
#pragma optimize("",on)
