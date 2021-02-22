/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/ludm.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <luainterface.hpp>
#include <udm.hpp>

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

static luabind::object get_property_value(lua_State *l,::udm::LinkedPropertyWrapper val)
{
	if(::udm::is_numeric_type(val->type))
	{
		auto tag = ::udm::get_numeric_tag(val->type);
		return std::visit([&](auto tag){return luabind::object{l,val->GetValue<decltype(tag)::type>()};},tag);
	}
	if(::udm::is_generic_type(val->type))
	{
		auto tag = ::udm::get_generic_tag(val->type);
		return std::visit([&](auto tag) {
			if constexpr(std::is_same_v<decltype(tag)::type,udm::Nil>)
				return luabind::object{};
			else
				return luabind::object{l,val->GetValue<decltype(tag)::type>()};
		},tag);
	}
	switch(val->type)
	{
	case ::udm::Type::String:
		return luabind::object{l,val->GetValue<::udm::String>()};
	case ::udm::Type::Utf8String:
	{
		auto &utf8String = val->GetValue<::udm::Utf8String>();
		DataStream ds {utf8String.data.data(),static_cast<uint32_t>(utf8String.data.size())};
		return luabind::object{l,ds};
	}
	case ::udm::Type::Blob:
	{
		auto &blob = val->GetValue<::udm::Blob>();
		DataStream ds {blob.data.data(),static_cast<uint32_t>(blob.data.size())};
		return luabind::object{l,ds};
	}
	case ::udm::Type::BlobLz4:
	{
		auto &blobLz4 = val->GetValue<::udm::BlobLz4>();
		auto t = luabind::newtable(l);
		t[1] = blobLz4.uncompressedSize;
		t[2] = DataStream{blobLz4.compressedData.data(),static_cast<uint32_t>(blobLz4.compressedData.size())};
		return t;
	}
	case ::udm::Type::HdrColor:
	{
		auto &hdrCol = val->GetValue<::udm::HdrColor>();
		auto t = luabind::newtable(l);
		for(auto i=decltype(hdrCol.size()){0u};i<hdrCol.size();++i)
			t[i +1] = hdrCol[i];
		return t;
	}
	}
	static_assert(umath::to_integral(::udm::Type::Count) == 29,"Update this when types have been added or removed!");
}

void Lua::udm::register_library(Lua::Interface &lua)
{
	auto &modUdm = lua.RegisterLibrary("udm",std::unordered_map<std::string,int(*)(lua_State*)>{
		{"load",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string fileName = Lua::CheckString(l,1);
			std::string err;
			auto udmData = ::udm::Data::Load(fileName,err);
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
			std::string fileName = Lua::CheckString(l,1);
			std::string err;
			auto udmData = ::udm::Data::Open(fileName,err);
			if(udmData == nullptr)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,err);
				return 2;
			}
			Lua::Push(l,udmData);
			return 1;
		})},
		{"load",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			auto &f = Lua::Check<LFile>(l,1);
			std::string err;
			auto udmData = ::udm::Data::Load(f.GetHandle(),err);
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
			auto &f = Lua::Check<LFile>(l,1);
			std::string err;
			auto udmData = ::udm::Data::Open(f.GetHandle(),err);
			if(udmData == nullptr)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,err);
				return 2;
			}
			Lua::Push(l,udmData);
			return 1;
		})},
		{"create",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) {
			std::string assetType = Lua::CheckString(l,1);
			auto assetVersion = Lua::CheckInt(l,2);
			std::string err;
			auto udmData = ::udm::Data::Create(assetType,assetVersion,err);
			if(udmData == nullptr)
			{
				Lua::PushBool(l,false);
				Lua::PushString(l,err);
				return 2;
			}
			Lua::Push(l,udmData);
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
	cdEl.def("Set",static_cast<void(*)(lua_State*,::udm::Element&,const std::string&,::udm::Type,luabind::object)>([](lua_State *l,::udm::Element &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,el.fromProperty[key],type,o);
	}));
	cdEl.def("Get",static_cast<luabind::object(*)(lua_State*,::udm::Element&,const std::string&)>([](lua_State *l,::udm::Element &el,const std::string &key) -> luabind::object {
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
		std::string err;
		auto result = udmData.Save(fileName,err);
		Lua::PushBool(l,result);
		if(result == false)
			Lua::PushString(l,err);
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &fileName) {
		std::string err;
		auto result = udmData.SaveAscii(fileName,err);
		Lua::PushBool(l,result);
		if(result == false)
			Lua::PushString(l,err);
	}));
	cdData.def("Save",static_cast<void(*)(lua_State*,::udm::Data&,LFile&)>([](lua_State *l,::udm::Data &udmData,LFile &f) {
		std::string err;
		auto fptr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
		if(fptr == nullptr)
			return;
		auto result = udmData.Save(fptr,err);
		Lua::PushBool(l,result);
		if(result == false)
			Lua::PushString(l,err);
	}));
	cdData.def("SaveAscii",static_cast<void(*)(lua_State*,::udm::Data&,LFile&)>([](lua_State *l,::udm::Data &udmData,LFile &f) {
		std::string err;
		auto fptr = std::dynamic_pointer_cast<VFilePtrInternalReal>(f.GetHandle());
		if(fptr == nullptr)
			return;
		auto result = udmData.SaveAscii(fptr,err);
		Lua::PushBool(l,result);
		if(result == false)
			Lua::PushString(l,err);
	}));
	cdData.def("ToAscii",static_cast<std::string(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> std::string {
		std::stringstream ss;
		udmData.ToAscii(ss);
		return ss.str();
	}));
	cdData.def("GetAssetData",static_cast<::udm::AssetData(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> ::udm::AssetData {return udmData.GetAssetData();}));
	cdData.def("LoadProperty",static_cast<void(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &path) {
		std::string err;
		auto prop = udmData.LoadProperty(path,err);
		if(prop == nullptr)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,err);
			return;
		}
		Lua::Push(l,prop);
	}));
	cdData.def("GetRootElement",static_cast<::udm::Element*(*)(lua_State*,::udm::Data&)>([](lua_State *l,::udm::Data &udmData) -> ::udm::Element* {
		return &udmData.GetRootElement();
	}));
	cdData.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::Data&,const std::string&)>([](lua_State *l,::udm::Data &udmData,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return udmData.GetRootElement()[key];
	}));
	modUdm[cdData];

	auto cdPropWrap = luabind::class_<::udm::PropertyWrapper>("PropertyWrapper");
	cdPropWrap.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &key) -> ::udm::LinkedPropertyWrapper {
		return p[key];
	}));
	cdPropWrap.def("Get",static_cast<::udm::LinkedPropertyWrapper(*)(lua_State*,::udm::PropertyWrapper&,uint32_t)>([](lua_State *l,::udm::PropertyWrapper &p,uint32_t idx) -> ::udm::LinkedPropertyWrapper {
		return p[idx];
	}));
	cdPropWrap.def("Set",static_cast<void(*)(lua_State*,::udm::PropertyWrapper&,const std::string&,::udm::Type,luabind::object)>([](lua_State *l,::udm::PropertyWrapper &el,const std::string &key,::udm::Type type,luabind::object o) {
		set_property_value(l,el[key],type,o);
	}));
	cdPropWrap.def("Get",static_cast<luabind::object(*)(lua_State*,::udm::PropertyWrapper&,const std::string&)>([](lua_State *l,::udm::PropertyWrapper &p,const std::string &key) -> luabind::object {
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
	modUdm[cdLinkedPropWrap];

	auto cdAssetData = luabind::class_<::udm::AssetData,::udm::LinkedPropertyWrapper,::udm::PropertyWrapper>("AssetData");
	cdAssetData.def("GetAssetType",&::udm::AssetData::GetAssetType);
	cdAssetData.def("GetAssetVersion",&::udm::AssetData::GetAssetVersion);
	cdAssetData.def("SetAssetType",&::udm::AssetData::SetAssetType);
	cdAssetData.def("SetAssetVersion",&::udm::AssetData::SetAssetVersion);
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
		return get_property_value(l,prop);
	}));
	cdProp.def("SetValue",static_cast<void(*)(lua_State*,::udm::Property&,::udm::Type,luabind::object)>([](lua_State *l,::udm::Property &prop,::udm::Type type,luabind::object o) {
		set_property_value(l,prop,type,o);
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
