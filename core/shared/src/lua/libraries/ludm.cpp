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
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/converters/string_view_converter_t.hpp"
#include "pragma/lua/types/udm.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/util/util_game.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <sharedutils/util_path.hpp>
#include <luabind/iterator_policy.hpp>
#include <luainterface.hpp>
#include <udm.hpp>
#include <datasystem_vector.h>
#include <datasystem_color.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_ifile.hpp>
#include <fsys/ifile.hpp>

extern DLLNETWORK Engine *engine;
#pragma optimize("", off)
Lua::udm::LuaUdmArrayIterator::LuaUdmArrayIterator(::udm::PropertyWrapper &prop) : m_property {&prop} {}

template<typename T>
static bool is_udm_type(luabind::object &val)
{
	return luabind::object_cast_nothrow<T *>(val, static_cast<T *>(nullptr)) != nullptr;
}
static udm::Type determine_udm_type(luabind::object &val)
{
	auto t = luabind::type(val);
	switch(t) {
	case LUA_TNIL:
		return udm::Type::Nil;
	case LUA_TBOOLEAN:
		return udm::Type::Boolean;
	case LUA_TSTRING:
		return udm::Type::String;
	case LUA_TNUMBER:
		{
			auto v = luabind::object_cast<double>(val);
			if(static_cast<int64_t>(v) == v) {
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

void Lua::udm::table_to_udm(const Lua::tb<void> &t, ::udm::LinkedPropertyWrapper &udm)
{
	for(luabind::iterator it {t}, end; it != end; ++it) {
		luabind::object val = *it;
		std::string key = luabind::object_cast<std::string>(luabind::object {it.key()});
		if(luabind::type(val) == LUA_TTABLE) {
			auto *l = val.interpreter();
			auto len = Lua::GetObjectLength(l, val);
			uint32_t actualLen = 0;
			auto isArrayType = true;
			std::pair<int, luabind::detail::class_rep *> typeInfo {-1, nullptr};
			for(luabind::iterator it {val}, end; it != end; ++it) {
				auto t = luabind::type(*it);
				if(t != LUA_TUSERDATA) {
					if(typeInfo.second != nullptr) {
						isArrayType = false;
						break;
					}
					if(typeInfo.first == -1)
						typeInfo.first = t;
					else if(t != typeInfo.first) {
						isArrayType = false;
						break;
					}
					++actualLen;
					continue;
				}
				else {
					auto *crep = Lua::get_crep(val);
					if(!crep || typeInfo.first != -1) {
						isArrayType = false;
						break;
					}
					if(typeInfo.second == nullptr)
						typeInfo.second = crep;
					else if(crep != typeInfo.second) {
						isArrayType = false;
						break;
					}
				}
				++actualLen;
			}
			if(actualLen > 0 && isArrayType) {
				luabind::object first {val[1]};
				auto type = determine_udm_type(first);
				if(type != ::udm::Type::Invalid) {
					auto a = udm.AddArray(key, actualLen, type);
					::udm::visit(type, [actualLen, &a, &val](auto tag) {
						using T = typename decltype(tag)::type;
						constexpr auto type = ::udm::type_to_enum<T>();
						if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type)) {
							for(auto i = decltype(actualLen) {0u}; i < actualLen; ++i)
								a[i] = Lua::udm::cast_object<T>(luabind::object {val[i + 1]});
						}
					});
				}
				continue;
			}
			auto udmSub = udm[key];
			table_to_udm(val, udmSub);
		}
		else {
			auto type = determine_udm_type(val);
			if(type != ::udm::Type::Invalid) {
				::udm::visit(type, [&udm, &key, &val](auto tag) {
					using T = typename decltype(tag)::type;
					constexpr auto type = ::udm::type_to_enum<T>();
					if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type))
						udm[key] = Lua::udm::cast_object<T>(val);
				});
			}
		}
	}
}
luabind::object Lua::udm::udm_to_value(lua_State *l, ::udm::LinkedPropertyWrapperArg udm)
{
	auto type = udm.GetType();
	if(::udm::is_array_type(type)) {
		auto t = luabind::newtable(l);
		for(uint32_t idx = 1; auto &val : udm)
			t[idx++] = udm_to_value(l, val);
		return t;
	}
	else if(type == ::udm::Type::Element) {
		auto t = luabind::newtable(l);
		for(auto &pair : const_cast<::udm::LinkedPropertyWrapper &>(udm).ElIt())
			t[pair.key] = udm_to_value(l, pair.property);
		return t;
	}
	return ::udm::visit(type, [l, &udm](auto tag) {
		using T = typename decltype(tag)::type;
		constexpr auto type = ::udm::type_to_enum<T>();
		if constexpr(type != ::udm::Type::Element && !::udm::is_array_type(type))
			return luabind::object {l, udm.GetValue<T>()};
		else
			return luabind::object {};
	});
}

static void data_block_to_udm(ds::Block &dataBlock, udm::LinkedPropertyWrapperArg udm)
{
	std::function<void(udm::LinkedPropertyWrapperArg, ds::Block &)> dataBlockToUdm = nullptr;
	dataBlockToUdm = [&dataBlockToUdm, &udm](udm::LinkedPropertyWrapperArg prop, ds::Block &block) {
		const_cast<udm::LinkedPropertyWrapper &>(prop).InitializeProperty();

		for(auto &pair : *block.GetData()) {
			auto &key = pair.first;
			auto &val = pair.second;
			if(val->IsBlock()) {
				auto &block = static_cast<ds::Block &>(*pair.second);

				auto &children = *block.GetData();
				auto n = children.size();
				uint32_t i = 0;
				auto isArray = true;
				while(i < n) {
					if(children.find(std::to_string(i)) == children.end()) {
						isArray = false;
						break;
					}
					++i;
				}

				if(isArray) {
					auto a = prop.AddArray(key, n, udm::Type::String);
					for(auto i = decltype(n) {0u}; i < n; ++i) {
						auto it = children.find(std::to_string(i));
						if(it == children.end())
							throw std::runtime_error {"Unknown error"};
						auto &val = it->second;
						if(val->IsBlock() || val->IsContainer())
							throw std::runtime_error {"Unknown error"};
						a[i] = static_cast<const ds::Value &>(*val).GetString();
					}
					continue;
				}

				dataBlockToUdm(prop[key], block);
				continue;
			}
			if(val->IsContainer()) {
				auto &container = static_cast<ds::Container &>(*pair.second);
				auto &children = container.GetBlocks();
				auto udmChildren = prop.AddArray(key, children.size());
				uint32_t idx = 0;
				for(auto &child : children) {
					if(child->IsContainer() || child->IsBlock())
						continue;
					auto *dsValue = dynamic_cast<ds::Value *>(pair.second.get());
					if(dsValue == nullptr)
						continue;
					udmChildren[idx++] = dsValue->GetString();
				}
				udmChildren.Resize(idx);
				continue;
			}
			auto *dsValue = dynamic_cast<ds::Value *>(val.get());
			assert(dsValue);
			if(dsValue) {
				auto *dsStr = dynamic_cast<ds::String *>(dsValue);
				if(dsStr)
					prop[key] = dsStr->GetString();
				auto *dsInt = dynamic_cast<ds::Int *>(dsValue);
				if(dsInt)
					prop[key] = dsInt->GetInt();
				auto *dsFloat = dynamic_cast<ds::Float *>(dsValue);
				if(dsFloat)
					prop[key] = dsFloat->GetFloat();
				auto *dsBool = dynamic_cast<ds::Bool *>(dsValue);
				if(dsBool)
					prop[key] = dsBool->GetBool();
				auto *dsVec = dynamic_cast<ds::Vector *>(dsValue);
				if(dsVec)
					prop[key] = dsVec->GetVector();
				auto *dsVec4 = dynamic_cast<ds::Vector4 *>(dsValue);
				if(dsVec4)
					prop[key] = dsVec4->GetVector4();
				auto *dsVec2 = dynamic_cast<ds::Vector2 *>(dsValue);
				if(dsVec2)
					prop[key] = dsVec2->GetVector2();
				auto *dsTex = dynamic_cast<ds::Texture *>(dsValue);
				if(dsTex)
					udm["textures"][key] = dsTex->GetString();
				auto *dsCol = dynamic_cast<ds::Color *>(dsValue);
				if(dsCol) {
					auto col = dsCol->GetColor();
					auto max = std::numeric_limits<udm::Srgba::value_type>::max();
					if(col.r <= max && col.g <= max && col.b <= max && col.a <= max)
						prop[key] = udm::Srgba {static_cast<uint8_t>(col.r), static_cast<uint8_t>(col.g), static_cast<uint8_t>(col.b), static_cast<uint8_t>(col.a)};
					else
						prop[key] = col.ToVector4();
				}
			}
		}
	};
	dataBlockToUdm(udm, dataBlock);
}

static std::string serialize(::udm::PropertyWrapper &p)
{
	auto data = udm::Data::Create();
	data->GetAssetData().GetData().Merge(p, ::udm::MergeFlags::DeepCopy);
	std::stringstream ss;
	data->ToAscii(ss, ::udm::AsciiSaveFlags::Default | ::udm::AsciiSaveFlags::DontCompressLz4Arrays);
	return ss.str();
}

static Lua::var<::udm::PProperty, std::pair<bool, std::string>> deserialize(lua_State *l, const std::string &str)
{
	auto memFile = std::make_unique<::ufile::MemoryFile>(reinterpret_cast<uint8_t *>(const_cast<char *>(str.data())), str.size());
	std::shared_ptr<::udm::Data> data = nullptr;
	try {
		data = ::udm::Data::Load(std::move(memFile));
	}
	catch(const udm::AsciiException &e) {
		return luabind::object {l, std::pair<bool, std::string> {false, e.what()}};
	}
	return luabind::object {l, data->GetAssetData().GetData().ClaimOwnership()};
}

static Lua::var<Lua::mult<bool, std::string>, ::udm::Data> create(lua_State *l, ::udm::PProperty rootProp, std::optional<std::string> assetType, std::optional<uint32_t> assetVersion)
{
	try {
		auto udmData = ::udm::Data::Create(assetType.has_value() ? *assetType : "", assetVersion.has_value() ? *assetVersion : 1);
		if(rootProp)
			udmData->GetAssetData().GetData() = rootProp;
		return luabind::object {l, udmData};
	}
	catch(const ::udm::Exception &e) {
		return Lua::mult<bool, std::string> {l, false, e.what()};
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

	auto isType = [&o]<typename T>() -> bool { return luabind::object_cast_nothrow<T *>(o, static_cast<T *>(nullptr)) != nullptr; };
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
static T &get_lua_object_udm_value(const luabind::object &o)
    requires(!std::is_arithmetic_v<T>)
{
	return *luabind::object_cast<T *>(o);
}

template<typename T>
static T get_lua_object_udm_value(const luabind::object &o)
    requires(std::is_arithmetic_v<T>)
{
	return Lua::udm::cast_object<T>(o);
}

template<typename T>
using lua_udm_underlying_numeric_type = std::conditional_t<std::is_same_v<T, bool>, uint8_t, udm::underlying_numeric_type<T>>;

template<typename T>
static lua_udm_underlying_numeric_type<T> get_numeric_component(const T &value, int32_t idx)
{
	if(idx >= udm::get_numeric_component_count(udm::type_to_enum<T>()))
		throw std::runtime_error {"Index " + std::to_string(idx) + " exceeds component count of UDM type '" + std::string {magic_enum::enum_name(udm::type_to_enum<T>())} + "'!"};
	if constexpr(std::is_same_v<T, bool>)
		return static_cast<uint8_t>(value);
	else
		return udm::get_numeric_component(value, idx);
}

template<typename T>
static void set_numeric_component(T &value, int32_t idx, lua_udm_underlying_numeric_type<T> compVal)
{
	if(idx >= udm::get_numeric_component_count(udm::type_to_enum<T>()))
		throw std::runtime_error {"Index " + std::to_string(idx) + " exceeds component count of UDM type '" + std::string {magic_enum::enum_name(udm::type_to_enum<T>())} + "'!"};
	udm::set_numeric_component(value, idx, compVal);
}

static Lua::type<uint32_t> get_numeric_component(lua_State *l, const luabind::object &value, int32_t idx, udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : determine_lua_object_udm_type(value);
	if(type == udm::Type::Invalid)
		return Lua::nil;
	return ::udm::visit_ng(type, [l, &value, idx](auto tag) {
		using T = typename decltype(tag)::type;
		using BaseType = lua_udm_underlying_numeric_type<T>;
		if constexpr(std::is_same_v<lua_udm_underlying_numeric_type<T>, void>)
			return Lua::nil;
		else
			return luabind::object {l, ::get_numeric_component<T>(get_lua_object_udm_value<T>(value), idx)};
	});
}

static luabind::object set_numeric_component(lua_State *l, const luabind::object &value, int32_t idx, udm::Type type, const Lua::udm_numeric &componentValue)
{
	type = (type != udm::Type::Invalid) ? type : determine_lua_object_udm_type(value);
	if(type == udm::Type::Invalid)
		return Lua::nil;
	return ::udm::visit_ng(type, [l, &value, idx, &componentValue](auto tag) {
		using T = typename decltype(tag)::type;
		using BaseType = lua_udm_underlying_numeric_type<T>;
		if constexpr(std::is_same_v<lua_udm_underlying_numeric_type<T>, void>)
			return Lua::nil;
		else if constexpr(std::is_arithmetic_v<T>)
			return luabind::object {l, componentValue};
		else {
			::set_numeric_component<T>(get_lua_object_udm_value<T>(value), idx, luabind::object_cast<BaseType>(componentValue));
			return value;
		}
	});
}

static Lua::udm_ng lerp_value(lua_State *l, const luabind::object &value0, const luabind::object &value1, float t, udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : determine_lua_object_udm_type(value0);
	if(type == udm::Type::Invalid)
		return Lua::nil;
	return ::udm::visit_ng(type, [l, &value0, &value1, t, type](auto tag) {
		using T = typename decltype(tag)::type;
		T valuer;
		::udm::lerp_value<T>(get_lua_object_udm_value<T>(value0), get_lua_object_udm_value<T>(value1), t, valuer, type);
		return luabind::object {l, valuer};
	});
}

static bool is_supported_array_value_type(::udm::Type valueType, ::udm::ArrayType arrayType)
{
	switch(arrayType) {
	case ::udm::ArrayType::Compressed:
		return ::udm::ArrayLz4::IsValueTypeSupported(valueType);
	default:
		return ::udm::Array::IsValueTypeSupported(valueType);
	}
	return false;
}

static bool is_same_element(lua_State *l, ::udm::LinkedPropertyWrapper &prop0, ::udm::LinkedPropertyWrapper &prop1)
{
	auto *el0 = prop0.GetValuePtr<::udm::Element>();
	auto *el1 = prop1.GetValuePtr<::udm::Element>();
	return (el0 && el1) && el0 == el1;
}

namespace Lua::udm {
	void register_types(Lua::Interface &lua, luabind::module_ &modUdm);
};

static bool compare_numeric_values(const Lua::udm_ng &ov0, const Lua::udm_ng &ov1, ::udm::Type type, double epsilon = 0.0001)
{
	if(::udm::is_numeric_type(type)) {
		return ::udm::visit<true, false, false>(type, [&ov0, &ov1, epsilon](auto tag) {
			using T = typename decltype(tag)::type;
			return umath::abs(luabind::object_cast<T>(ov0) - luabind::object_cast<T>(ov1)) < epsilon;
		});
	}
	return ::udm::visit<false, true, false>(type, [&ov0, &ov1, epsilon](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(!std::is_same_v<T, ::udm::Nil> && !std::is_same_v<T, ::udm::Transform> && !std::is_same_v<T, ::udm::ScaledTransform>) {
			constexpr auto type = ::udm::type_to_enum<T>();
			auto n = ::udm::get_numeric_component_count(type);
			auto *v0 = luabind::object_cast<T *>(ov0);
			auto *v1 = luabind::object_cast<T *>(ov1);
			for(auto i = decltype(n) {0u}; i < n; ++i) {
				auto f0 = ::get_numeric_component<T>(*v0, i);
				auto f1 = ::get_numeric_component<T>(*v1, i);
				if(umath::abs(f1 - f0) >= epsilon)
					return false;
			}
		}
		return true;
	});
}

void Lua::udm::register_library(Lua::Interface &lua)
{
	auto modUdm = luabind::module(lua.GetState(), "udm");
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
			auto udmData = ::util::load_udm_asset(std::make_unique<ufile::FileWrapper>(file.GetHandle()),&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("parse",+[](lua_State *l,const std::string &udmStr) -> Lua::var<Lua::mult<bool,std::string>,::udm::Data> {
			auto memFile = std::make_unique<ufile::MemoryFile>(reinterpret_cast<uint8_t*>(const_cast<char*>(udmStr.data())),udmStr.length());
			std::string err;
			auto udmData = ::util::load_udm_asset(std::move(memFile),&err);
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
				auto udmData = ::udm::Data::Open(std::make_unique<ufile::FileWrapper>(file.GetHandle()));
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
		luabind::def("is_convertible",static_cast<bool(*)(::udm::Type,::udm::Type)>(&::udm::is_convertible)),
		luabind::def("serialize",&::serialize),
		luabind::def("deserialize",&::deserialize),
		luabind::def("is_supported_array_value_type",&is_supported_array_value_type),
		luabind::def("is_supported_array_value_type",&is_supported_array_value_type,luabind::default_parameter_policy<2,::udm::ArrayType::Raw>{}),
		luabind::def("convert",+[](lua_State *l,const luabind::object &o0,::udm::Type t0,::udm::Type t1) -> luabind::object {
			return ::udm::visit<true,true,true>(t0,[l,&o0,t1](auto tag){
                using T0 = typename decltype(tag)::type;
				if constexpr(pragma::is_valid_component_property_type(::udm::type_to_enum<T0>()))
				{
					auto v0 = luabind::object_cast<T0>(o0);
					return ::udm::visit<true,true,true>(t1,[l,&v0](auto tag){
                        using T1 = typename decltype(tag)::type;
						if constexpr(::udm::is_convertible<T0,T1>())
						{
							auto v1 = ::udm::convert<T0,T1>(v0);
							return luabind::object{l,v1};
						}
						else
						{
							return Lua::nil;
						}
					});
				}
				else
				{
					return Lua::nil;
				}
			});
		}),
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
                using T = typename decltype(tag)::type;
				return ::udm::is_vector_type<T>;
			});
		}),
		luabind::def("is_matrix_type",+[](::udm::Type type) -> bool {
			return ::udm::visit<true,true,true>(type,[type](auto tag){
                using T = typename decltype(tag)::type;
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
		luabind::def("string_to_type",+[](const std::string &type) -> std::optional<::udm::Type> {
			return magic_enum::enum_cast<::udm::Type>(type);
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
		luabind::def("set_numeric_component",static_cast<luabind::object(*)(lua_State*,const luabind::object&,int32_t,::udm::Type, const Lua::udm_numeric&)>(&set_numeric_component)),
		luabind::def("lerp",+[](lua_State *l,const luabind::object &value0,const luabind::object &value1,float t) -> Lua::udm_ng {
			return lerp_value(l,value0,value1,t,::udm::Type::Invalid);
		}),
		luabind::def("lerp",static_cast<Lua::udm_ng(*)(lua_State*,const luabind::object&,const luabind::object&,float,::udm::Type)>(&::lerp_value)),
		luabind::def("get_format_type",+[](lua_State *l,const std::string &fileName) -> Lua::var<::udm::FormatType,Lua::mult<bool,std::string>> {
			std::string err;
			auto formatType = ::udm::Data::GetFormatType(fileName, err);
			if(!formatType)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*formatType};
		}),
		luabind::def("convert_udm_file_to_ascii",+[](lua_State *l,const std::string &fileName) -> Lua::var<std::string,Lua::mult<bool,std::string>> {
			std::string err;
			auto newFileName = ::util::convert_udm_file_to_ascii(fileName, err);
		    if(!newFileName)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*newFileName};
		}),
		luabind::def("convert_udm_file_to_binary",+[](lua_State *l,const std::string &fileName) -> Lua::var<std::string,Lua::mult<bool,std::string>> {
			std::string err;
			auto newFileName = ::util::convert_udm_file_to_binary(fileName, err);
		    if(!newFileName)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*newFileName};
		}),
		luabind::def("get_default_value",+[](lua_State *l,::udm::Type type) -> luabind::object {
			return ::udm::visit<true,true,true>(type,[l,type](auto tag){
                using T = typename decltype(tag)::type;
				if constexpr(!::udm::is_non_trivial_type(::udm::type_to_enum<T>()))
					return luabind::object{l,T{}};
				else
					return Lua::nil;
			});
		}),
		luabind::def("get_underlying_numeric_type",+[](lua_State *l,::udm::Type type) -> ::udm::Type {
			return ::udm::visit_ng(type,[l,type](auto tag){
                using T = typename decltype(tag)::type;
				return ::udm::type_to_enum<lua_udm_underlying_numeric_type<T>>();
			});
		}),
		luabind::def("is_same_element", &is_same_element),
		luabind::def("compare_numeric_values",&compare_numeric_values),
		luabind::def("compare_numeric_values",+[](const Lua::udm_ng &ov0, const Lua::udm_ng &ov1, ::udm::Type type) {
			return compare_numeric_values(ov0,ov1,type);
		})
		// luabind::def("compare_numeric_values",&compare_numeric_values,luabind::default_parameter_policy<4,0.0001>{}) // Currently not supported by clang compiler
	];

	Lua::RegisterLibraryEnums(lua.GetState(), "udm",
	  {
	    {"TYPE_NIL", umath::to_integral(::udm::Type::Nil)},
	    {"TYPE_STRING", umath::to_integral(::udm::Type::String)},
	    {"TYPE_UTF8_STRING", umath::to_integral(::udm::Type::Utf8String)},
	    {"TYPE_INT8", umath::to_integral(::udm::Type::Int8)},
	    {"TYPE_UINT8", umath::to_integral(::udm::Type::UInt8)},
	    {"TYPE_INT16", umath::to_integral(::udm::Type::Int16)},
	    {"TYPE_UINT16", umath::to_integral(::udm::Type::UInt16)},
	    {"TYPE_INT32", umath::to_integral(::udm::Type::Int32)},
	    {"TYPE_UINT32", umath::to_integral(::udm::Type::UInt32)},
	    {"TYPE_INT64", umath::to_integral(::udm::Type::Int64)},
	    {"TYPE_UINT64", umath::to_integral(::udm::Type::UInt64)},
	    {"TYPE_FLOAT", umath::to_integral(::udm::Type::Float)},
	    {"TYPE_DOUBLE", umath::to_integral(::udm::Type::Double)},
	    {"TYPE_BOOLEAN", umath::to_integral(::udm::Type::Boolean)},
	    {"TYPE_VECTOR2", umath::to_integral(::udm::Type::Vector2)},
	    {"TYPE_VECTOR2I", umath::to_integral(::udm::Type::Vector2i)},
	    {"TYPE_VECTOR3", umath::to_integral(::udm::Type::Vector3)},
	    {"TYPE_VECTOR3I", umath::to_integral(::udm::Type::Vector3i)},
	    {"TYPE_VECTOR4", umath::to_integral(::udm::Type::Vector4)},
	    {"TYPE_VECTOR4I", umath::to_integral(::udm::Type::Vector4i)},
	    {"TYPE_QUATERNION", umath::to_integral(::udm::Type::Quaternion)},
	    {"TYPE_EULER_ANGLES", umath::to_integral(::udm::Type::EulerAngles)},
	    {"TYPE_SRGBA", umath::to_integral(::udm::Type::Srgba)},
	    {"TYPE_HDR_COLOR", umath::to_integral(::udm::Type::HdrColor)},
	    {"TYPE_TRANSFORM", umath::to_integral(::udm::Type::Transform)},
	    {"TYPE_SCALED_TRANSFORM", umath::to_integral(::udm::Type::ScaledTransform)},
	    {"TYPE_MAT4", umath::to_integral(::udm::Type::Mat4)},
	    {"TYPE_MAT3X4", umath::to_integral(::udm::Type::Mat3x4)},
	    {"TYPE_BLOB", umath::to_integral(::udm::Type::Blob)},
	    {"TYPE_BLOB_LZ4", umath::to_integral(::udm::Type::BlobLz4)},
	    {"TYPE_ELEMENT", umath::to_integral(::udm::Type::Element)},
	    {"TYPE_ARRAY", umath::to_integral(::udm::Type::Array)},
	    {"TYPE_ARRAY_LZ4", umath::to_integral(::udm::Type::ArrayLz4)},
	    {"TYPE_REFERENCE", umath::to_integral(::udm::Type::Reference)},
	    {"TYPE_STRUCT", umath::to_integral(::udm::Type::Struct)},
	    {"TYPE_HALF", umath::to_integral(::udm::Type::Half)},
	    {"TYPE_COUNT", umath::to_integral(::udm::Type::Count)},
	    {"TYPE_INVALID", umath::to_integral(::udm::Type::Invalid)},

	    {"ARRAY_TYPE_COMPRESSED", umath::to_integral(::udm::ArrayType::Compressed)},
	    {"ARRAY_TYPE_RAW", umath::to_integral(::udm::ArrayType::Raw)},

	    {"MERGE_FLAG_NONE", umath::to_integral(::udm::MergeFlags::None)},
	    {"MERGE_FLAG_BIT_OVERWRITE_EXISTING", umath::to_integral(::udm::MergeFlags::OverwriteExisting)},
	    {"MERGE_FLAG_BIT_DEEP_COPY", umath::to_integral(::udm::MergeFlags::DeepCopy)},

	    {"ASCII_SAVE_FLAG_NONE", umath::to_integral(::udm::AsciiSaveFlags::None)},
	    {"ASCII_SAVE_FLAG_BIT_INCLUDE_HEADER", umath::to_integral(::udm::AsciiSaveFlags::IncludeHeader)},
	    {"ASCII_SAVE_FLAG_BIT_DONT_COMPRESS_LZ4_ARRAYS", umath::to_integral(::udm::AsciiSaveFlags::DontCompressLz4Arrays)},
	    {"ASCII_SAVE_FLAG_DEFAULT", umath::to_integral(::udm::AsciiSaveFlags::Default)},
		
	    {"FORMAT_TYPE_BINARY", umath::to_integral(::udm::FormatType::Binary)},
	    {"FORMAT_TYPE_ASCII", umath::to_integral(::udm::FormatType::Ascii)},
	  });
	static_assert(umath::to_integral(::udm::Type::Count) == 36, "Update this list when types have been added or removed!");

	register_types(lua, modUdm);
}
