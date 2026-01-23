// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :scripting.lua.libraries.udm;

Lua::udm::LuaUdmArrayIterator::LuaUdmArrayIterator(::udm::PropertyWrapper &prop) : m_property {&prop} {}

template<typename T>
static bool is_udm_type(const luabind::object &val)
{
	return luabind::object_cast_nothrow<T *>(val, static_cast<T *>(nullptr)) != nullptr;
}
udm::Type Lua::udm::determine_udm_type(const luabind::object &val)
{
	auto type = static_cast<Type>(luabind::type(val));
	switch(type) {
	case Type::Nil:
		return ::udm::Type::Nil;
	case Type::Bool:
		return ::udm::Type::Boolean;
	case Type::String:
		return ::udm::Type::String;
	case Type::Number:
		{
			auto v = luabind::object_cast<double>(val);
			if(static_cast<int64_t>(v) == v) {
				if(v < 0.0)
					return ::udm::Type::UInt32;
				return ::udm::Type::Int32;
			}
			return ::udm::Type::Float;
		}
	case Type::UserData:
		break;
	default:
		return ::udm::Type::Invalid;
	}

	if(is_udm_type<::udm::Vector2>(val))
		return ::udm::Type::Vector2;
	if(is_udm_type<::udm::Vector3>(val))
		return ::udm::Type::Vector3;
	if(is_udm_type<::udm::Vector4>(val))
		return ::udm::Type::Vector4;
	if(is_udm_type<::udm::Vector2i>(val))
		return ::udm::Type::Vector2i;
	if(is_udm_type<::udm::Vector3i>(val))
		return ::udm::Type::Vector3i;
	if(is_udm_type<::udm::Vector4i>(val))
		return ::udm::Type::Vector4i;
	if(is_udm_type<::udm::Quaternion>(val))
		return ::udm::Type::Quaternion;
	if(is_udm_type<::udm::EulerAngles>(val))
		return ::udm::Type::EulerAngles;
	if(is_udm_type<::udm::Srgba>(val))
		return ::udm::Type::Srgba;
	if(is_udm_type<::udm::HdrColor>(val))
		return ::udm::Type::HdrColor;
	if(is_udm_type<::udm::Transform>(val))
		return ::udm::Type::Transform;
	if(is_udm_type<::udm::ScaledTransform>(val))
		return ::udm::Type::ScaledTransform;
	if(is_udm_type<::udm::Mat4>(val))
		return ::udm::Type::Mat4;
	if(is_udm_type<::udm::Mat3x4>(val))
		return ::udm::Type::Mat3x4;
	if(is_udm_type<::udm::String>(val))
		return ::udm::Type::String;
	if(is_udm_type<::udm::Utf8String>(val))
		return ::udm::Type::Utf8String;
	if(is_udm_type<::udm::Blob>(val))
		return ::udm::Type::Blob;
	if(is_udm_type<::udm::BlobLz4>(val))
		return ::udm::Type::BlobLz4;
	if(is_udm_type<::udm::Element>(val))
		return ::udm::Type::Element;
	if(is_udm_type<::udm::Array>(val))
		return ::udm::Type::Array;
	if(is_udm_type<::udm::ArrayLz4>(val))
		return ::udm::Type::ArrayLz4;
	if(is_udm_type<::udm::Reference>(val))
		return ::udm::Type::Reference;
	if(is_udm_type<::udm::Struct>(val))
		return ::udm::Type::Struct;
	return ::udm::Type::Invalid;
}

void Lua::udm::table_to_udm(const tb<void> &t, ::udm::LinkedPropertyWrapper &udm)
{
	for(luabind::iterator it {t}, end; it != end; ++it) {
		luabind::object val = *it;
		std::string key = luabind::object_cast<std::string>(luabind::object {it.key()});
		if(static_cast<Type>(luabind::type(val)) == Type::Table) {
			auto *l = val.interpreter();
			auto len = GetObjectLength(l, val);
			uint32_t actualLen = 0;
			auto isArrayType = true;
			std::pair<Type, luabind::detail::class_rep *> typeInfo {Type::None, nullptr};
			for(luabind::iterator it {val}, end; it != end; ++it) {
				auto t = static_cast<Type>(luabind::type(*it));
				if(t != Type::UserData) {
					if(typeInfo.second != nullptr) {
						isArrayType = false;
						break;
					}
					if(typeInfo.first == Type::None)
						typeInfo.first = t;
					else if(t != typeInfo.first) {
						isArrayType = false;
						break;
					}
					++actualLen;
					continue;
				}
				else {
					auto *crep = get_crep(val);
					if(!crep || typeInfo.first != Type::None) {
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
								a[i] = udm::cast_object<T>(luabind::object {val[i + 1]});
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
						udm[key] = udm::cast_object<T>(val);
				});
			}
		}
	}
}
luabind::object Lua::udm::udm_to_value(lua::State *l, ::udm::LinkedPropertyWrapperArg udm)
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

static void data_block_to_udm(pragma::datasystem::Block &dataBlock, udm::LinkedPropertyWrapperArg udm)
{
	std::function<void(udm::LinkedPropertyWrapperArg, pragma::datasystem::Block &)> dataBlockToUdm = nullptr;
	dataBlockToUdm = [&dataBlockToUdm, &udm](udm::LinkedPropertyWrapperArg prop, pragma::datasystem::Block &block) {
		const_cast<udm::LinkedPropertyWrapper &>(prop).InitializeProperty();

		for(auto &pair : *block.GetData()) {
			auto &key = pair.first;
			auto &val = pair.second;
			if(val->IsBlock()) {
				auto &block = static_cast<pragma::datasystem::Block &>(*pair.second);

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
						a[i] = static_cast<const pragma::datasystem::Value &>(*val).GetString();
					}
					continue;
				}

				dataBlockToUdm(prop[key], block);
				continue;
			}
			if(val->IsContainer()) {
				auto &container = static_cast<pragma::datasystem::Container &>(*pair.second);
				auto &children = container.GetBlocks();
				auto udmChildren = prop.AddArray(key, children.size());
				uint32_t idx = 0;
				for(auto &child : children) {
					if(child->IsContainer() || child->IsBlock())
						continue;
					auto *dsValue = dynamic_cast<pragma::datasystem::Value *>(pair.second.get());
					if(dsValue == nullptr)
						continue;
					udmChildren[idx++] = dsValue->GetString();
				}
				udmChildren.Resize(idx);
				continue;
			}
			auto *dsValue = dynamic_cast<pragma::datasystem::Value *>(val.get());
			assert(dsValue);
			if(dsValue) {
				auto *dsStr = dynamic_cast<pragma::datasystem::String *>(dsValue);
				if(dsStr)
					prop[key] = dsStr->GetString();
				auto *dsInt = dynamic_cast<pragma::datasystem::Int *>(dsValue);
				if(dsInt)
					prop[key] = dsInt->GetInt();
				auto *dsFloat = dynamic_cast<pragma::datasystem::Float *>(dsValue);
				if(dsFloat)
					prop[key] = dsFloat->GetFloat();
				auto *dsBool = dynamic_cast<pragma::datasystem::Bool *>(dsValue);
				if(dsBool)
					prop[key] = dsBool->GetBool();
				auto *dsVec = dynamic_cast<pragma::datasystem::Vector *>(dsValue);
				if(dsVec)
					prop[key] = dsVec->GetVector();
				auto *dsVec4 = dynamic_cast<pragma::datasystem::Vector4 *>(dsValue);
				if(dsVec4)
					prop[key] = dsVec4->GetVector4();
				auto *dsVec2 = dynamic_cast<pragma::datasystem::Vector2 *>(dsValue);
				if(dsVec2)
					prop[key] = dsVec2->GetVector2();
				auto *dsTex = dynamic_cast<pragma::datasystem::Texture *>(dsValue);
				if(dsTex)
					udm["textures"][key] = dsTex->GetString();
				auto *dsCol = dynamic_cast<pragma::datasystem::Color *>(dsValue);
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

static std::string serialize(udm::PropertyWrapper &p)
{
	auto data = udm::Data::Create();
	data->GetAssetData().GetData().Merge(p, udm::MergeFlags::DeepCopy);
	std::stringstream ss;
	data->ToAscii(ss, udm::AsciiSaveFlags::Default | udm::AsciiSaveFlags::DontCompressLz4Arrays);
	return ss.str();
}

static Lua::var<udm::PProperty, std::pair<bool, std::string>> deserialize(lua::State *l, const std::string &str)
{
	auto memFile = std::make_unique<ufile::MemoryFile>(reinterpret_cast<uint8_t *>(const_cast<char *>(str.data())), str.size());
	std::shared_ptr<udm::Data> data = nullptr;
	try {
		data = udm::Data::Load(std::move(memFile));
	}
	catch(const udm::AsciiException &e) {
		return luabind::object {l, std::pair<bool, std::string> {false, e.what()}};
	}
	return luabind::object {l, data->GetAssetData().GetData().ClaimOwnership()};
}

static Lua::var<Lua::mult<bool, std::string>, udm::Data> create(lua::State *l, udm::PProperty rootProp, std::optional<std::string> assetType, std::optional<uint32_t> assetVersion)
{
	try {
		auto udmData = udm::Data::Create(assetType.has_value() ? *assetType : "", assetVersion.has_value() ? *assetVersion : 1);
		if(rootProp)
			udmData->GetAssetData().GetData() = rootProp;
		return luabind::object {l, udmData};
	}
	catch(const udm::Exception &e) {
		return Lua::mult<bool, std::string> {l, false, e.what()};
	}
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

static Lua::type<uint32_t> get_numeric_component(lua::State *l, const luabind::object &value, int32_t idx, udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : Lua::udm::determine_udm_type(value);
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

static luabind::object set_numeric_component(lua::State *l, const luabind::object &value, int32_t idx, udm::Type type, const Lua::udm_numeric &componentValue)
{
	type = (type != udm::Type::Invalid) ? type : Lua::udm::determine_udm_type(value);
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

static Lua::udm_ng lerp_value(lua::State *l, const luabind::object &value0, const luabind::object &value1, float t, udm::Type type)
{
	type = (type != udm::Type::Invalid) ? type : Lua::udm::determine_udm_type(value0);
	if(type == udm::Type::Invalid)
		return Lua::nil;
	return ::udm::visit_ng(type, [l, &value0, &value1, t, type](auto tag) {
		using T = typename decltype(tag)::type;
		T valuer;
		udm::lerp_value<T>(get_lua_object_udm_value<T>(value0), get_lua_object_udm_value<T>(value1), t, valuer, type);
		return luabind::object {l, valuer};
	});
}

static bool is_supported_array_value_type(udm::Type valueType, udm::ArrayType arrayType)
{
	switch(arrayType) {
	case udm::ArrayType::Compressed:
		return udm::ArrayLz4::IsValueTypeSupported(valueType);
	default:
		return udm::Array::IsValueTypeSupported(valueType);
	}
	return false;
}

static bool is_same_element(lua::State *l, udm::LinkedPropertyWrapper &prop0, udm::LinkedPropertyWrapper &prop1)
{
	auto *el0 = prop0.GetValuePtr<udm::Element>();
	auto *el1 = prop1.GetValuePtr<udm::Element>();
	return (el0 && el1) && el0 == el1;
}

namespace Lua::udm {
	void register_types(Interface &lua, luabind::module_ &modUdm);
};

static bool compare_numeric_values(const Lua::udm_ng &ov0, const Lua::udm_ng &ov1, udm::Type type, double epsilon = 0.0001)
{
	if(::udm::is_numeric_type(type)) {
		return ::udm::visit<true, false, false>(type, [&ov0, &ov1, epsilon](auto tag) {
			using T = typename decltype(tag)::type;
			return pragma::math::abs(luabind::object_cast<T>(ov0) - luabind::object_cast<T>(ov1)) < epsilon;
		});
	}
	return ::udm::visit<false, true, false>(type, [&ov0, &ov1, epsilon](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(!std::is_same_v<T, udm::Nil> && !std::is_same_v<T, udm::Transform> && !std::is_same_v<T, udm::ScaledTransform>) {
			constexpr auto type = udm::type_to_enum<T>();
			auto n = udm::get_numeric_component_count(type);
			auto *v0 = luabind::object_cast<T *>(ov0);
			auto *v1 = luabind::object_cast<T *>(ov1);
			for(auto i = decltype(n) {0u}; i < n; ++i) {
				auto f0 = ::get_numeric_component<T>(*v0, i);
				auto f1 = ::get_numeric_component<T>(*v1, i);
				if(pragma::math::abs(f1 - f0) >= epsilon)
					return false;
			}
		}
		return true;
	});
}

void Lua::udm::register_library(Interface &lua)
{
	auto modUdm = luabind::module(lua.GetState(), "udm");
	modUdm[(
		luabind::def("load",+[](lua::State *l,const std::string &fileName) -> var<mult<bool,std::string>,::udm::Data> {
			std::string err;
			auto udmData = pragma::util::load_udm_asset(fileName,&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("load",+[](lua::State *l,LFile &file) -> var<mult<bool,std::string>,::udm::Data> {
			if(!file.GetHandle())
				return Lua::mult<bool,std::string>{l,false,"Invalid file handle!"};
			std::string err;
			auto udmData = pragma::util::load_udm_asset(std::make_unique<ufile::FileWrapper>(file.GetHandle()),&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("parse",+[](lua::State *l,const std::string &udmStr) -> var<mult<bool,std::string>,::udm::Data> {
			auto memFile = std::make_unique<ufile::MemoryFile>(reinterpret_cast<uint8_t*>(const_cast<char*>(udmStr.data())),udmStr.length());
			std::string err;
			auto udmData = pragma::util::load_udm_asset(std::move(memFile),&err);
			if(udmData == nullptr)
				return Lua::mult<bool,std::string>{l,false,std::move(err)};
			return luabind::object{l,udmData};
		}),
		luabind::def("open",+[](lua::State *l,const std::string &fileName) -> var<mult<bool,std::string>,::udm::Data> {
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
		luabind::def("open",+[](lua::State *l,LFile &file) -> var<mult<bool,std::string>,::udm::Data> {
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
		luabind::def("create",+[](lua::State *l,::udm::PProperty rootProp,std::optional<std::string> assetType) -> var<mult<bool,std::string>,::udm::Data> {
			return create(l,rootProp,assetType,{});
		}),
		luabind::def("create",+[](lua::State *l,::udm::PProperty rootProp) -> var<mult<bool,std::string>,::udm::Data> {
			return create(l,rootProp,{},{});
		}),
		luabind::def("create",+[](lua::State *l) -> var<mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,{},{});
		}),
		luabind::def("create",+[](lua::State *l,std::optional<std::string> assetType,std::optional<uint32_t> assetVersion) -> var<mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,assetType,assetVersion);
		}),
		luabind::def("create",+[](lua::State *l,std::optional<std::string> assetType) -> var<mult<bool,std::string>,::udm::Data> {
			return create(l,nullptr,assetType,{});
		}),
		luabind::def("create_property",static_cast<::udm::PProperty(*)(::udm::Type)>(&::udm::Property::Create)),
		luabind::def("create_element",+[](lua::State *l) -> ::udm::PProperty {
			return ::udm::Property::Create(::udm::Type::Element);
		}),
		luabind::def("compress_lz4",+[](lua::State *l,pragma::util::DataStream ds) -> pragma::util::DataStream {
			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto lz4Blob = ::udm::compress_lz4_blob(ds->GetData(),ds->GetInternalSize());
			ds->SetOffset(offset);

			auto t = luabind::newtable(l);
			t[1] = lz4Blob.uncompressedSize;
			pragma::util::DataStream dsCompressed {static_cast<uint32_t>(lz4Blob.compressedData.size())};
			dsCompressed->Write(lz4Blob.compressedData.data(),lz4Blob.compressedData.size());
			t[2] = dsCompressed;
			return dsCompressed;
		}),
		luabind::def("decompress_lz4",+[](tb<void> t) -> pragma::util::DataStream {
			uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1],uint64_t{});
			pragma::util::DataStream ds = luabind::object_cast_nothrow<pragma::util::DataStream>(t[2],pragma::util::DataStream{});

			auto offset = ds->GetOffset();
			ds->SetOffset(0);
			auto blob = ::udm::decompress_lz4_blob(ds->GetData(),ds->GetInternalSize(),uncompressedSize);
			ds->SetOffset(offset);

			pragma::util::DataStream dsDecompressed {static_cast<uint32_t>(blob.data.size())};
			dsDecompressed->Write(blob.data.data(),blob.data.size());
			return dsDecompressed;
		}),
		luabind::def("data_block_to_udm",&data_block_to_udm),
		luabind::def("data_file_to_udm",+[](const std::string &fileName) -> bool {
			std::string rpath;
			if(pragma::fs::find_absolute_path(fileName,rpath) == false)
				return false;
			auto f = pragma::fs::open_file(fileName.c_str(), pragma::fs::FileMode::Read);
			if(f == nullptr)
				return false;
			pragma::fs::File fp {f};
			auto root = pragma::datasystem::System::ReadData(fp);
			if(root == nullptr)
				return false;
			ufile::remove_extension_from_filename(rpath);
			rpath += ".udm";
			pragma::fs::find_relative_path(rpath, rpath);
			auto fout = pragma::fs::open_file<pragma::fs::VFilePtrReal>(rpath,pragma::fs::FileMode::Write);
			if(fout == nullptr)
				return false;
			auto udmData = ::udm::Data::Create();
			data_block_to_udm(*root,udmData->GetAssetData().GetData());
			return udmData->SaveAscii(fout);
		}),
		luabind::def("debug_test",&::udm::Data::DebugTest),
		luabind::def("define_struct",+[](tb<void> t) -> ::udm::StructDescription {
			::udm::StructDescription structDesc {};
			auto n = GetObjectLength(t.interpreter(),t);
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
		luabind::def("serialize",&serialize),
		luabind::def("deserialize",&deserialize),
		luabind::def("is_supported_array_value_type",&is_supported_array_value_type),
		luabind::def("is_supported_array_value_type",&is_supported_array_value_type,luabind::default_parameter_policy<2,::udm::ArrayType::Raw>{}),
		luabind::def("convert",+[](lua::State *l,const luabind::object &o0,::udm::Type t0,::udm::Type t1) -> luabind::object {
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
							return nil;
						}
					});
				}
				else
				{
					return nil;
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
			static_assert(pragma::math::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
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
			static_assert(pragma::math::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
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
		luabind::def("get_class_type",+[](lua::State *l,::udm::Type type) -> luabind::object {
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
			static_assert(pragma::math::to_integral(::udm::Type::Count) == 36,"Update this when types have been added or removed!");
			return nil;
		}),
		luabind::def("get_numeric_component",+[](lua::State *l,const luabind::object &value,int32_t idx) -> type<uint32_t> {
			return get_numeric_component(l,value,idx,::udm::Type::Invalid);
		}),
		luabind::def("get_numeric_component",static_cast<type<uint32_t>(*)(lua::State*,const luabind::object&,int32_t,::udm::Type)>(&get_numeric_component)),
		luabind::def("set_numeric_component",static_cast<luabind::object(*)(lua::State*,const luabind::object&,int32_t,::udm::Type, const udm_numeric&)>(&set_numeric_component)),
		luabind::def("lerp",+[](lua::State *l,const luabind::object &value0,const luabind::object &value1,float t) -> udm_ng {
			return lerp_value(l,value0,value1,t,::udm::Type::Invalid);
		}),
		luabind::def("lerp",static_cast<udm_ng(*)(lua::State*,const luabind::object&,const luabind::object&,float,::udm::Type)>(&lerp_value)),
		luabind::def("get_format_type",+[](lua::State *l,const std::string &fileName) -> var<::udm::FormatType,mult<bool,std::string>> {
			std::string err;
			auto formatType = ::udm::Data::GetFormatType(fileName, err);
			if(!formatType)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*formatType};
		}),
		luabind::def("convert_udm_file_to_ascii",+[](lua::State *l,const std::string &fileName) -> var<std::string,mult<bool,std::string>> {
			std::string err;
			auto newFileName = pragma::util::convert_udm_file_to_ascii(fileName, err);
		    if(!newFileName)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*newFileName};
		}),
		luabind::def("convert_udm_file_to_binary",+[](lua::State *l,const std::string &fileName) -> var<std::string,mult<bool,std::string>> {
			std::string err;
			auto newFileName = pragma::util::convert_udm_file_to_binary(fileName, err);
		    if(!newFileName)
				return luabind::object{l,std::pair<bool,std::string>{false,err}};
			return luabind::object{l,*newFileName};
		}),
		luabind::def("get_default_value",+[](lua::State *l,::udm::Type type) -> luabind::object {
			return ::udm::visit<true,true,true>(type,[l,type](auto tag){
                using T = typename decltype(tag)::type;
				if constexpr(!::udm::is_non_trivial_type(::udm::type_to_enum<T>()))
					return luabind::object{l,T{}};
				else
					return nil;
			});
		}),
		luabind::def("get_underlying_numeric_type",+[](lua::State *l,::udm::Type type) -> ::udm::Type {
			return ::udm::visit_ng(type,[l,type](auto tag){
                using T = typename decltype(tag)::type;
				return ::udm::type_to_enum<lua_udm_underlying_numeric_type<T>>();
			});
		}),
		luabind::def("is_same_element", &is_same_element),
		luabind::def("compare_numeric_values",&compare_numeric_values),
		luabind::def("compare_numeric_values",+[](const udm_ng &ov0, const udm_ng &ov1, ::udm::Type type) {
			return compare_numeric_values(ov0,ov1,type);
		}),
		luabind::def("is_animatable_type", +[](::udm::Type type) -> bool {
			if(pragma::math::to_integral(type) >= pragma::math::to_integral(::udm::Type::Count))
				return false;
			return panima::is_animatable_type(type);
		})
		// luabind::def("compare_numeric_values",&compare_numeric_values,luabind::default_parameter_policy<4,0.0001>{}) // Currently not supported by clang compiler
	)];

	RegisterLibraryEnums(lua.GetState(), "udm",
	  {
	    {"TYPE_NIL", pragma::math::to_integral(::udm::Type::Nil)},
	    {"TYPE_STRING", pragma::math::to_integral(::udm::Type::String)},
	    {"TYPE_UTF8_STRING", pragma::math::to_integral(::udm::Type::Utf8String)},
	    {"TYPE_INT8", pragma::math::to_integral(::udm::Type::Int8)},
	    {"TYPE_UINT8", pragma::math::to_integral(::udm::Type::UInt8)},
	    {"TYPE_INT16", pragma::math::to_integral(::udm::Type::Int16)},
	    {"TYPE_UINT16", pragma::math::to_integral(::udm::Type::UInt16)},
	    {"TYPE_INT32", pragma::math::to_integral(::udm::Type::Int32)},
	    {"TYPE_UINT32", pragma::math::to_integral(::udm::Type::UInt32)},
	    {"TYPE_INT64", pragma::math::to_integral(::udm::Type::Int64)},
	    {"TYPE_UINT64", pragma::math::to_integral(::udm::Type::UInt64)},
	    {"TYPE_FLOAT", pragma::math::to_integral(::udm::Type::Float)},
	    {"TYPE_DOUBLE", pragma::math::to_integral(::udm::Type::Double)},
	    {"TYPE_BOOLEAN", pragma::math::to_integral(::udm::Type::Boolean)},
	    {"TYPE_VECTOR2", pragma::math::to_integral(::udm::Type::Vector2)},
	    {"TYPE_VECTOR2I", pragma::math::to_integral(::udm::Type::Vector2i)},
	    {"TYPE_VECTOR3", pragma::math::to_integral(::udm::Type::Vector3)},
	    {"TYPE_VECTOR3I", pragma::math::to_integral(::udm::Type::Vector3i)},
	    {"TYPE_VECTOR4", pragma::math::to_integral(::udm::Type::Vector4)},
	    {"TYPE_VECTOR4I", pragma::math::to_integral(::udm::Type::Vector4i)},
	    {"TYPE_QUATERNION", pragma::math::to_integral(::udm::Type::Quaternion)},
	    {"TYPE_EULER_ANGLES", pragma::math::to_integral(::udm::Type::EulerAngles)},
	    {"TYPE_SRGBA", pragma::math::to_integral(::udm::Type::Srgba)},
	    {"TYPE_HDR_COLOR", pragma::math::to_integral(::udm::Type::HdrColor)},
	    {"TYPE_TRANSFORM", pragma::math::to_integral(::udm::Type::Transform)},
	    {"TYPE_SCALED_TRANSFORM", pragma::math::to_integral(::udm::Type::ScaledTransform)},
	    {"TYPE_MAT4", pragma::math::to_integral(::udm::Type::Mat4)},
	    {"TYPE_MAT3X4", pragma::math::to_integral(::udm::Type::Mat3x4)},
	    {"TYPE_BLOB", pragma::math::to_integral(::udm::Type::Blob)},
	    {"TYPE_BLOB_LZ4", pragma::math::to_integral(::udm::Type::BlobLz4)},
	    {"TYPE_ELEMENT", pragma::math::to_integral(::udm::Type::Element)},
	    {"TYPE_ARRAY", pragma::math::to_integral(::udm::Type::Array)},
	    {"TYPE_ARRAY_LZ4", pragma::math::to_integral(::udm::Type::ArrayLz4)},
	    {"TYPE_REFERENCE", pragma::math::to_integral(::udm::Type::Reference)},
	    {"TYPE_STRUCT", pragma::math::to_integral(::udm::Type::Struct)},
	    {"TYPE_HALF", pragma::math::to_integral(::udm::Type::Half)},
	    {"TYPE_COUNT", pragma::math::to_integral(::udm::Type::Count)},
	    {"TYPE_INVALID", pragma::math::to_integral(::udm::Type::Invalid)},

	    {"ARRAY_TYPE_COMPRESSED", pragma::math::to_integral(::udm::ArrayType::Compressed)},
	    {"ARRAY_TYPE_RAW", pragma::math::to_integral(::udm::ArrayType::Raw)},

	    {"MERGE_FLAG_NONE", pragma::math::to_integral(::udm::MergeFlags::None)},
	    {"MERGE_FLAG_BIT_OVERWRITE_EXISTING", pragma::math::to_integral(::udm::MergeFlags::OverwriteExisting)},
	    {"MERGE_FLAG_BIT_DEEP_COPY", pragma::math::to_integral(::udm::MergeFlags::DeepCopy)},

	    {"ASCII_SAVE_FLAG_NONE", pragma::math::to_integral(::udm::AsciiSaveFlags::None)},
	    {"ASCII_SAVE_FLAG_BIT_INCLUDE_HEADER", pragma::math::to_integral(::udm::AsciiSaveFlags::IncludeHeader)},
	    {"ASCII_SAVE_FLAG_BIT_DONT_COMPRESS_LZ4_ARRAYS", pragma::math::to_integral(::udm::AsciiSaveFlags::DontCompressLz4Arrays)},
	    {"ASCII_SAVE_FLAG_DEFAULT", pragma::math::to_integral(::udm::AsciiSaveFlags::Default)},

	    {"FORMAT_TYPE_BINARY", pragma::math::to_integral(::udm::FormatType::Binary)},
	    {"FORMAT_TYPE_ASCII", pragma::math::to_integral(::udm::FormatType::Ascii)},
	  });
	static_assert(pragma::math::to_integral(::udm::Type::Count) == 36, "Update this list when types have been added or removed!");

	register_types(lua, modUdm);
}
