// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.libraries.udm;

template<class TPropertyWrapper>
void clear(TPropertyWrapper &prop)
{
	if constexpr(std::is_same_v<TPropertyWrapper, udm::Element>)
		prop.children.clear();
	else {
		auto *el = prop.template GetValuePtr<udm::Element>();
		if(el) {
			el->children.clear();
			return;
		}
		auto *a = prop.template GetValuePtr<udm::Array>();
		if(a)
			a->Resize(0);
	}
}

template<class TPropertyWrapper, class TClass, typename T>
void prop_set_basic_type(lua::State *l, TClass &p, const std::string &key, const T &v)
{
	static_cast<TPropertyWrapper>(p)[key] = v;
}

template<class TPropertyWrapper, class TClass, typename T>
void prop_set_basic_type_indexed(lua::State *l, TClass &p, int32_t idx, const T &v)
{
	if(idx < 0)
		return;
	static_cast<TPropertyWrapper>(p)[idx] = v;
}

static udm::LinkedPropertyWrapper x_get(lua::State *l, udm::Element &p, const std::string &key) { return static_cast<udm::PropertyWrapper &>(p)[key]; }
static udm::LinkedPropertyWrapper x_get(lua::State *l, udm::PropertyWrapper &p, const std::string &key) { return static_cast<udm::PropertyWrapper &>(p)[key]; }
static udm::LinkedPropertyWrapper x_get(lua::State *l, udm::LinkedPropertyWrapper &p, const std::string &key) { return static_cast<udm::LinkedPropertyWrapper &>(p)[key]; }
static udm::LinkedPropertyWrapper x_get(lua::State *l, udm::Property &p, const std::string &key) { return static_cast<udm::PropertyWrapper>(p)[key]; }

static udm::LinkedPropertyWrapper i_get(lua::State *l, udm::Element &p, int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<udm::PropertyWrapper &>(p)[idx];
}
static udm::LinkedPropertyWrapper i_get(lua::State *l, udm::LinkedPropertyWrapper &p, int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<udm::LinkedPropertyWrapper &>(p)[idx];
}
static udm::LinkedPropertyWrapper i_get(lua::State *l, udm::PropertyWrapper &p, int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<udm::PropertyWrapper &>(p)[idx];
}
static udm::LinkedPropertyWrapper i_get(lua::State *l, udm::Property &p, int32_t idx)
{
	if(idx < 0)
		return {};
	return static_cast<udm::PropertyWrapper>(p)[idx];
}

static luabind::object get_blob_array_values(lua::State *l, const udm::PropertyWrapper &p, udm::Type type)
{
	auto t = luabind::newtable(l);
	auto vs = [l, &p, &type, &t](auto tag) {
		using T = typename decltype(tag)::type;
		auto blobData = p.GetBlobData(type);
		auto *ptr = reinterpret_cast<T *>(blobData.data.data());
		auto n = blobData.data.size() / sizeof(T);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			t[i + 1] = ptr[i];
	};
	if(::udm::is_ng_type(type))
		::udm::visit_ng(type, vs);
	return t;
}

static void set_blob_array_values(lua::State *l, const udm::PropertyWrapper &p, const std::string &path, udm::Type type, luabind::tableT<void> t, udm::Type blobType = udm::Type::BlobLz4)
{
	if(blobType != udm::Type::Blob && blobType != udm::Type::BlobLz4)
		return;
	auto n = Lua::GetObjectLength(l, 4);
	auto vs = [l, &p, &type, &t, &path, blobType, n](auto tag) {
		using T = typename decltype(tag)::type;
		if(blobType == udm::Type::Blob) {
			auto &blob = p.Add(path, udm::Type::Blob).GetValue<udm::Blob>();
			blob.data.resize(sizeof(T) * n);
			auto *p = reinterpret_cast<T *>(blob.data.data());
			for(auto i = decltype(n) {0u}; i < n; ++i) {
				*p = luabind::object_cast_nothrow<T>(t[i + 1], T {});
				++p;
			}
			return;
		}
		if constexpr(std::is_same_v<T, bool>) {
			// Boolean vectors in c++ are special, so we'll have to handle them separately...
			std::vector<uint8_t> values {};
			values.resize(n);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				values[i] = luabind::object_cast_nothrow<bool>(t[i + 1], false);
			p[path] = udm::compress_lz4_blob(reinterpret_cast<uint8_t *>(values.data()), values.size() * sizeof(values.front()));
		}
		else {
			std::vector<T> values {};
			values.resize(n);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				values[i] = luabind::object_cast_nothrow<T>(t[i + 1], T {});
			p[path] = udm::compress_lz4_blob(reinterpret_cast<uint8_t *>(values.data()), values.size() * sizeof(values.front()));
		}
	};
	if(::udm::is_ng_type(type))
		::udm::visit_ng(type, vs);
}

static luabind::object get_array_values(lua::State *l, udm::LinkedPropertyWrapperArg p, std::optional<udm::Type> type)
{
	auto t = luabind::newtable(l);
	auto size = p.GetSize();
	auto *pArray = p.GetValuePtr<udm::Array>();
	if(size == 0 || pArray == nullptr)
		return t;

	if(!type.has_value()) {
		int32_t idx = 1;
		for(auto prop : p)
			t[idx++] = prop;
		return t;
	}

	auto vs = [&t, pArray, size](auto tag) {
		using T = typename decltype(tag)::type;
		auto *p = static_cast<T *>(pArray->GetValues());
		for(auto i = decltype(size) {0u}; i < size; ++i) {
			t[i + 1] = *p;
			++p;
		}
	};
	if(::udm::is_numeric_type(*type) || ::udm::is_generic_type(*type))
		udm::visit_ng(*type, vs);
	else if(::udm::is_non_trivial_type(*type)) {
		udm::visit<false, false, true>(*type, [&vs, size, pArray, &p, &t](auto tag) {
			using T = typename decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T, udm::String>)
				vs(tag);
			else if constexpr(std::is_same_v<T, udm::Element>) {
				for(auto i = decltype(size) {0u}; i < size; ++i)
					t[i + 1] = p[i];
			}
		});
	}
	return t;
}

static luabind::object get_array_value(lua::State *l, udm::LinkedPropertyWrapperArg p, int32_t idx, std::optional<udm::Type> type)
{
	if(idx < 0)
		return Lua::nil;
	auto size = p.GetSize();
	auto *pArray = p.GetValuePtr<udm::Array>();
	if(idx >= size || pArray == nullptr)
		return Lua::nil;

	if(!type.has_value())
		type = pArray->GetValueType();

	auto vs = [l, idx, pArray, size](auto tag) {
		using T = typename decltype(tag)::type;
		auto *p = static_cast<T *>(pArray->GetValues());
		return luabind::object {l, p[idx]};
	};
	if(::udm::is_numeric_type(*type) || ::udm::is_generic_type(*type))
		return udm::visit_ng(*type, vs);
	else if(::udm::is_non_trivial_type(*type)) {
		return udm::visit<false, false, true>(*type, [l, &vs, size, pArray, &p, idx](auto tag) -> luabind::object {
			using T = typename decltype(tag)::type;
			// TODO: Add support for other non-trivial types
			if constexpr(std::is_same_v<T, udm::String>)
				return vs(tag);
			else if constexpr(std::is_same_v<T, udm::Element>)
				return luabind::object {l, p[idx]};
			return Lua::nil;
		});
	}
	return Lua::nil;
}

static void remove_array(udm::PropertyWrapper &p, const std::string &name)
{
	auto *el = p.GetValuePtr<udm::Element>();
	if(!el)
		return;
	auto it = el->children.find(name);
	if(it != el->children.end())
		el->children.erase(it);
}

template<typename T>
concept is_assignable_type = !std::is_same_v<T, udm::Element> && !std::is_same_v<T, udm::Utf8String> && !std::is_same_v<T, udm::Array> && !std::is_same_v<T, udm::ArrayLz4>;
static void insert_array_value(lua::State *l, udm::PropertyWrapper &p, uint32_t idx, const luabind::object &o)
{
	auto *a = p.GetValuePtr<udm::Array>();
	if(!a)
		return;
	auto arrayType = a->GetArrayType();
	udm::visit(a->GetValueType(), [&a, idx, &o, arrayType](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(is_assignable_type<T>) {
			if(arrayType == udm::ArrayType::Raw) {
				if constexpr(udm::Array::IsValueTypeSupported(udm::type_to_enum<T>()))
					a->InsertValue<T>(idx, Lua::udm::cast_object<T>(o));
			}
			else if constexpr(udm::ArrayLz4::IsValueTypeSupported(udm::type_to_enum<T>()))
				a->InsertValue<T>(idx, Lua::udm::cast_object<T>(o));
		}
	});
}
static void push_array_value(lua::State *l, udm::PropertyWrapper &p, const luabind::object &o)
{
	auto *a = p.GetValuePtr<udm::Array>();
	if(!a)
		return;
	insert_array_value(l, p, a->GetSize(), o);
}
bool Lua::udm::set_array_value(lua::State *l, ::udm::Array &a, int32_t idx, const luabind::object &o)
{
	if(idx < 0 || idx >= a.GetSize())
		return false;
	auto arrayType = a.GetArrayType();
	::udm::visit(a.GetValueType(), [&a, idx, &o, arrayType](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(is_assignable_type<T>) {
			if(arrayType == ::udm::ArrayType::Raw) {
				if constexpr(::udm::Array::IsValueTypeSupported(::udm::type_to_enum<T>())) {
					auto *pVal = static_cast<T *>(a.GetValues());
					pVal[idx] = cast_object<T>(o);
				}
			}
			else if constexpr(::udm::ArrayLz4::IsValueTypeSupported(::udm::type_to_enum<T>())) {
				auto *pVal = static_cast<T *>(a.GetValues());
				pVal[idx] = cast_object<T>(o);
			}
		}
	});
	return true;
}
static luabind::object get_children(lua::State *l, const udm::PropertyWrapper &p)
{
	auto t = luabind::newtable(l);
	auto *el = p.GetValuePtr<udm::Element>();
	if(el == nullptr)
		return t;
	auto *linked = const_cast<udm::PropertyWrapper &>(p).GetLinked();
	if(linked) {
		for(auto pair : linked->ElIt())
			t[std::string {pair.key}] = (*linked)[pair.key];
	}
	else {
		for(auto it = p.begin_el(); it != p.end_el(); ++it) {
			auto &pair = *it;
			t[std::string {pair.key}] = p[pair.key];
		}
	}
	return t;
}
static udm::Blob data_stream_to_blob(pragma::util::DataStream &ds)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(), ds->GetData(), data.size());
	ds->SetOffset(offset);
	return udm::Blob {std::move(data)};
}

static udm::BlobLz4 data_stream_to_lz4_blob(pragma::util::DataStream &ds, uint64_t uncompressedSize)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(), ds->GetData(), data.size());
	ds->SetOffset(offset);
	return udm::BlobLz4 {std::move(data), uncompressedSize};
}

static udm::Utf8String data_stream_to_utf8(pragma::util::DataStream &ds)
{
	auto offset = ds->GetOffset();
	ds->SetOffset(0);
	std::vector<uint8_t> data {};
	data.resize(ds->GetInternalSize());
	memcpy(data.data(), ds->GetData(), data.size());
	ds->SetOffset(offset);
	return udm::Utf8String {std::move(data)};
}
static void set_property_value(lua::State *l, udm::LinkedPropertyWrapper p, udm::Type type, luabind::object o, int32_t idx, bool nil = false)
{
	if(nil) {
		p = udm::Nil {};
		return;
	}
	if(::udm::is_numeric_type(type)) {
		::udm::visit<true, false, false>(type, [l, idx, &p](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(std::is_same_v<T, udm::Half>)
				p = udm::Half {static_cast<float>(Lua::CheckNumber(l, idx))};
			else
				p = Lua::udm::cast_object<T>(luabind::object {luabind::from_stack(l, idx)});
		});
	}
	if(::udm::is_generic_type(type)) {
		::udm::visit<false, true, false>(type, [l, idx, &p](auto tag) {
			if constexpr(std::is_same_v<typename decltype(tag)::type, udm::Nil>)
				return;
			else
				p = static_cast<typename decltype(tag)::type &>(Lua::Check<typename decltype(tag)::type>(l, idx));
		});
	}
	switch(type) {
	case udm::Type::String:
		p = static_cast<udm::String>(Lua::CheckString(l, idx));
		break;
	case udm::Type::Utf8String:
		p = data_stream_to_utf8(Lua::Check<pragma::util::DataStream>(l, idx));
		break;
	case udm::Type::Reference:
		p = Lua::Check<udm::Reference>(l, idx);
		break;
	case udm::Type::Struct:
		p = Lua::Check<udm::Struct>(l, idx);
		break;
	case udm::Type::Blob:
		p = data_stream_to_blob(Lua::Check<pragma::util::DataStream>(l, idx));
		break;
	case udm::Type::BlobLz4:
		{
			Lua::CheckTable(l, idx);
			luabind::table<> t {luabind::from_stack(l, idx)};
			uint64_t uncompressedSize = luabind::object_cast_nothrow<uint64_t>(t[1], uint64_t {});
			pragma::util::DataStream ds = luabind::object_cast_nothrow<pragma::util::DataStream>(t[2], pragma::util::DataStream {});
			p = data_stream_to_lz4_blob(ds, uncompressedSize);
			break;
		}
	}
	static_assert(pragma::math::to_integral(udm::Type::Count) == 36, "Update this when types have been added or removed!");
}
template<typename T>
static bool is_type(luabind::object &o)
{
	try {
		luabind::object_cast<T *>(o);
	}
	catch(const std::exception &e) {
		return false;
	}
	return true;
}
static void set_property_value(lua::State *l, udm::LinkedPropertyWrapper p, luabind::object o)
{
	auto type = static_cast<Lua::Type>(luabind::type(o));
	auto udmType = udm::Type::Nil;
	switch(type) {
	case Lua::Type::Bool:
		udmType = udm::Type::Boolean;
		break;
	case Lua::Type::String:
		udmType = udm::Type::String;
		break;
	case Lua::Type::Number:
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
			else if(is_type<udm::Property>(o)) {
				auto prop = luabind::object_cast<udm::PProperty>(o);
				if(!prop) {
					p = udm::Nil {};
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
	set_property_value(l, p, udmType, o, 3);
}
template<typename T>
static void set_array_values(udm::Array &a, udm::Type type, luabind::tableT<void> t, size_t size, udm::ArrayType arrayType)
{
	a.Resize(size);
	auto *pVal = static_cast<T *>(a.GetValues());
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		*pVal = Lua::udm::cast_object_nothrow<T>(luabind::object {t[i + 1]});
		++pVal;
	}
}
template<typename T>
static void set_array_values(udm::PropertyWrapper &p, const std::string &name, udm::Type type, luabind::tableT<void> t, size_t size, udm::ArrayType arrayType)
{
	remove_array(p, name);
	auto a = p.AddArray(name, size, type, arrayType);
	set_array_values<T>(a.GetValue<udm::Array>(), type, t, size, arrayType);
}
static void set_array_values(lua::State *l, udm::Array &a, udm::Type type, luabind::tableT<void> t, uint32_t tIdx, udm::ArrayType arrayType = udm::ArrayType::Raw)
{
	auto size = Lua::GetObjectLength(l, tIdx);
	auto vs = [&t, &a, type, tIdx, arrayType, size](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(is_assignable_type<T>) {
			if(arrayType == udm::ArrayType::Raw) {
				if constexpr(udm::Array::IsValueTypeSupported(udm::type_to_enum<T>()))
					set_array_values<T>(a, type, t, size, arrayType);
			}
			else if constexpr(udm::ArrayLz4::IsValueTypeSupported(udm::type_to_enum<T>()))
				set_array_values<T>(a, type, t, size, arrayType);
		}
	};
	udm::visit(type, vs);
}
void Lua::udm::set_array_values(lua::State *l, ::udm::Array &a, luabind::tableT<void> t, uint32_t tIdx) { ::set_array_values(l, a, a.GetValueType(), t, tIdx, a.GetArrayType()); }
static void set_array_values(lua::State *l, udm::PropertyWrapper &p, const std::string &name, udm::Type type, luabind::tableT<void> t, udm::ArrayType arrayType = udm::ArrayType::Raw)
{
	auto size = Lua::GetObjectLength(l, 4);
	auto a = p.AddArray(name, size, type, arrayType);
	set_array_values(l, a.GetValue<udm::Array>(), type, t, 4, arrayType);
}

static void set_array_values(udm::PropertyWrapper &p, const std::string &name, udm::StructDescription &strct, uint32_t count, pragma::util::DataStream &ds, udm::ArrayType arrayType)
{
	remove_array(p, name);
	p.AddArray(name, strct, ds->GetData(), count, arrayType);
}

static luabind::object get_property_value(lua::State *l, udm::Type type, void *ptr)
{
	auto vs = [l, ptr](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(std::is_same_v<T, udm::Nil>)
			return luabind::object {};
		else {
			auto &v = *static_cast<T *>(ptr);
			if constexpr(std::is_same_v<T, udm::Half>)
				return luabind::object {l, static_cast<float>(v)};
			return luabind::object {l, v};
		}
	};
	if(::udm::is_ng_type(type))
		return ::udm::visit_ng(type, vs);
	switch(type) {
	case udm::Type::String:
		return luabind::object {l, *static_cast<udm::String *>(ptr)};
	case udm::Type::Srgba:
		return luabind::object {l, *static_cast<udm::Srgba *>(ptr)};
	case udm::Type::HdrColor:
		return luabind::object {l, *static_cast<udm::HdrColor *>(ptr)};
	case udm::Type::Reference:
		return luabind::object {l, static_cast<udm::Reference *>(ptr)};
	case udm::Type::Struct:
		return luabind::object {l, static_cast<udm::Struct *>(ptr)};
	case udm::Type::Utf8String:
		{
			auto &utf8String = *static_cast<udm::Utf8String *>(ptr);
			pragma::util::DataStream ds {utf8String.data.data(), static_cast<uint32_t>(utf8String.data.size())};
			return luabind::object {l, ds};
		}
	case udm::Type::Blob:
		{
			auto &blob = *static_cast<udm::Blob *>(ptr);
			pragma::util::DataStream ds {blob.data.data(), static_cast<uint32_t>(blob.data.size())};
			return luabind::object {l, ds};
		}
	case udm::Type::BlobLz4:
		{
			auto &blobLz4 = *static_cast<udm::BlobLz4 *>(ptr);
			auto t = luabind::newtable(l);
			t[1] = blobLz4.uncompressedSize;
			t[2] = pragma::util::DataStream {blobLz4.compressedData.data(), static_cast<uint32_t>(blobLz4.compressedData.size())};
			return t;
		}
	}
	static_assert(pragma::math::to_integral(udm::Type::Count) == 36, "Update this when types have been added or removed!");
	return {};
}
static luabind::object get_property_value(lua::State *l, const udm::PropertyWrapper &val)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type type;
	auto *ptr = val.GetValuePtr(type);
	if(ptr == nullptr)
		return {};
	if(type == udm::Type::Element) {
		auto *linked = val.GetLinked();
		if(linked)
			return luabind::object {l, *const_cast<udm::LinkedPropertyWrapper *>(linked)};
		return luabind::object {l, *const_cast<udm::PropertyWrapper *>(&val)};
	}
	return get_property_value(l, type, ptr);
}
static luabind::object get_property_value(lua::State *l, const udm::PropertyWrapper &val, udm::Type type)
{
	if(!static_cast<bool>(val))
		return {};
	udm::Type valType;
	auto *ptr = val.GetValuePtr(valType);
	if(ptr == nullptr)
		return {};
	if(valType == type)
		return get_property_value(l, val);
	if(!udm::is_convertible(valType, type))
		return {};
	auto vs = [l, ptr, &val, type](auto tag) {
		using T = typename decltype(tag)::type;
		auto v = val.ToValue<T>();
		if(!v.has_value())
			return luabind::object {};
		return get_property_value(l, type, &*v);
	};
	return udm::visit(type, vs);
}
static luabind::object get_property_value(lua::State *l, const udm::PropertyWrapper &val, int32_t idx)
{
	if(!static_cast<bool>(val))
		return {};
	auto *a = val.GetValuePtr<udm::Array>();
	if(!a || idx < 0 || idx >= a->GetSize())
		return Lua::nil;
	return get_property_value(l, a->GetValueType(), a->GetValuePtr(idx));
}

template<class T, class TPropertyWrapper, class TClassDef>
void register_property_methods(TClassDef &classDef)
{
	classDef.def(
	          "Add", +[](lua::State *l, T &p, const std::string &path) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).Add(path); })
	  .def(
	    "Add", +[](lua::State *l, T &p, const std::string &path, udm::Type type) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).Add(path, type); })
	  .def(
	    "AddAssetData", +[](lua::State *l, T &p, const std::string &path) -> udm::AssetData { return udm::AssetData {static_cast<TPropertyWrapper>(p).Add(path)}; })
	  .def(
	    "ToAssetData", +[](lua::State *l, T &prop) -> udm::AssetData { return udm::AssetData {static_cast<TPropertyWrapper>(prop)}; })
	  //.def("Get",+[](lua::State *l,T &p,const std::string &key) -> ::udm::LinkedPropertyWrapper {
	  //	return static_cast<TPropertyWrapper>(p)[key];
	  //})
	  .def(
	    "Get",
	    +[](lua::State *l, T &p, const std::string &key) -> udm::LinkedPropertyWrapper {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return x_get(l, *lp, key);
		    return x_get(l, p, key);
	    })
	  .def(
	    "Get",
	    +[](lua::State *l, T &p, int32_t idx) -> udm::LinkedPropertyWrapper {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return i_get(l, *lp, idx);
		    return i_get(l, p, idx);
	    })
	  //.def("Get",+[](lua::State *l,T &p,uint32_t idx) -> ::udm::LinkedPropertyWrapper {
	  //	return static_cast<TPropertyWrapper>(p)[idx];
	  //})
	  .def(
	    "GetArrayValuesFromBlob", +[](lua::State *l, T &p, udm::Type type) -> luabind::object { return get_blob_array_values(l, static_cast<TPropertyWrapper>(p), type); })
	  .def(
	    "AddBlobFromArrayValues", +[](lua::State *l, T &p, const std::string &path, udm::Type type, luabind::tableT<void> t, udm::Type blobType) { return set_blob_array_values(l, static_cast<TPropertyWrapper>(p), path, type, t, blobType); })
	  .def(
	    "AddBlobFromArrayValues", +[](lua::State *l, T &p, const std::string &path, udm::Type type, luabind::tableT<void> t) { return set_blob_array_values(l, static_cast<TPropertyWrapper>(p), path, type, t); })
	  .def(
	    "AddValueRange",
	    +[](lua::State *l, T &p, uint32_t startIndex, uint32_t count) {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::Array>();
		    if(!a)
			    return;
		    a->AddValueRange(startIndex, count);
	    })
	  .def(
	    "RemoveValueRange",
	    +[](lua::State *l, T &p, uint32_t startIndex, uint32_t count) {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::Array>();
		    if(!a)
			    return;
		    a->RemoveValueRange(startIndex, count);
	    })
	  .def(
	    "GetBlobData",
	    +[](lua::State *l, T &p) {
		    auto type = udm::Type::Nil;
		    auto vs = [l, &p, &type](auto tag) {
			    using T2 = typename decltype(tag)::type;
			    return static_cast<TPropertyWrapper>(p).GetBlobData(type);
		    };
		    udm::Blob blob;
		    if(::udm::is_ng_type(type))
			    blob = udm::visit_ng(type, vs);
		    else if(::udm::is_non_trivial_type(type))
			    return; // TODO
		    pragma::util::DataStream ds {static_cast<uint32_t>(blob.data.size())};
		    ds->Write(blob.data.data(), blob.data.size());
		    ds->SetOffset(0);
		    Lua::Push(l, ds);
		    Lua::Push(l, type);
	    })
	  .def(
	    "GetSize", +[](lua::State *l, T &p) -> uint32_t { return static_cast<TPropertyWrapper>(p).GetSize(); })
	  .def(
	    "GetValueType",
	    +[](lua::State *l, T &p) -> luabind::object {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::Array>();
		    if(!a)
			    return luabind::object {};
		    return luabind::object {l, pragma::math::to_integral(a->GetValueType())};
	    })
	  .def(
	    "SetValueType",
	    +[](lua::State *l, T &p, udm::Type valueType) {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::Array>();
		    if(!a)
			    return;
		    a->SetValueType(valueType);
	    })
	  .def(
	    "GetArrayValues",
	    +[](lua::State *l, T &p, udm::Type type) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_values(l, *lp, type);
		    return get_array_values(l, udm::LinkedPropertyWrapper {tp}, type);
	    })
	  .def(
	    "GetArrayValues",
	    +[](lua::State *l, T &p) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_values(l, (*lp), {});
		    return get_array_values(l, tp, {});
	    })
	  .def(
	    "GetArrayValues",
	    +[](lua::State *l, T &p, const std::string &name, udm::Type type) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_values(l, (*lp)[name], type);
		    return get_array_values(l, tp[name], type);
	    })
	  .def(
	    "GetArrayValues",
	    +[](lua::State *l, T &p, const std::string &name) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_values(l, (*lp)[name], {});
		    return get_array_values(l, tp[name], {});
	    })
	  .def(
	    "GetArrayValue",
	    +[](lua::State *l, T &p, int32_t idx, udm::Type type) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_value(l, *lp, idx, type);
		    return get_array_value(l, udm::LinkedPropertyWrapper {tp}, idx, type);
	    })
	  .def(
	    "GetArrayValue",
	    +[](lua::State *l, T &p, int32_t idx) -> luabind::object {
		    TPropertyWrapper tp = static_cast<TPropertyWrapper>(p);
		    auto *lp = tp.GetLinked();
		    if(lp)
			    return get_array_value(l, (*lp), idx, {});
		    return get_array_value(l, tp, idx, {});
	    })
	  .def(
	    "SetArrayValues",
	    +[](lua::State *l, T &p, udm::Type type, luabind::tableT<void> t) {
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    set_array_values(l, tmp.template GetValue<udm::Array>(), type, t, 3);
	    })
	  .def(
	    "SetArrayValues",
	    +[](lua::State *l, T &p, udm::Type type, luabind::tableT<void> t, udm::Type arrayType) {
		    if(arrayType != udm::Type::Array && arrayType != udm::Type::ArrayLz4)
			    Lua::Error(l, "Invalid array type '" + std::string {::udm::enum_type_to_ascii(arrayType)} + "'!");
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    set_array_values(l, tmp.template GetValue<udm::Array>(), type, t, 3, (arrayType == udm::Type::ArrayLz4) ? udm::ArrayType::Compressed : udm::ArrayType::Raw);
	    })
	  .def(
	    "SetArrayValues",
	    +[](lua::State *l, T &p, const std::string &name, udm::Type type, luabind::tableT<void> t) {
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    set_array_values(l, tmp, name, type, t);
	    })
	  .def(
	    "SetArrayValues",
	    +[](lua::State *l, T &p, const std::string &name, udm::Type type, luabind::tableT<void> t, udm::Type arrayType) {
		    if(arrayType != udm::Type::Array && arrayType != udm::Type::ArrayLz4)
			    Lua::Error(l, "Invalid array type '" + std::string {::udm::enum_type_to_ascii(arrayType)} + "'!");
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    set_array_values(l, tmp, name, type, t, (arrayType == udm::Type::ArrayLz4) ? udm::ArrayType::Compressed : udm::ArrayType::Raw);
	    })
	  .def(
	    "SetArrayValues",
	    +[](lua::State *l, T &p, const std::string &name, udm::StructDescription &strct, uint32_t count, pragma::util::DataStream &ds, udm::Type arrayType) {
		    if(arrayType != udm::Type::Array && arrayType != udm::Type::ArrayLz4)
			    Lua::Error(l, "Invalid array type '" + std::string {::udm::enum_type_to_ascii(arrayType)} + "'!");
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    set_array_values(tmp, name, strct, count, ds, (arrayType == udm::Type::ArrayLz4) ? udm::ArrayType::Compressed : udm::ArrayType::Raw);
	    })
	  .def(
	    "InsertValue",
	    +[](lua::State *l, T &p, uint32_t idx, const luabind::object &o) {
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    insert_array_value(l, tmp, idx, o);
	    })
	  .def(
	    "InsertValue",
	    +[](lua::State *l, T &p, const luabind::object &o) {
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    push_array_value(l, tmp, o);
	    })
	  .def(
	    "GetChildren", +[](lua::State *l, T &p) -> luabind::object { return get_children(l, static_cast<TPropertyWrapper>(p)); })
	  .def(
	    "GetChildren", +[](lua::State *l, T &p, const std::string &key) -> luabind::object { return get_children(l, static_cast<TPropertyWrapper>(p)[key]); })
	  .def(
	    "GetChildCount", +[](lua::State *l, T &p) -> uint32_t { return static_cast<TPropertyWrapper>(p).GetChildCount(); })
	  .def(
	    "GetChildCount", +[](lua::State *l, T &p, const std::string &key) -> uint32_t { return static_cast<TPropertyWrapper>(p)[key].GetChildCount(); })
	  .def(
	    "Resize", +[](lua::State *l, T &p, uint32_t size) { static_cast<TPropertyWrapper>(p).Resize(size); })
	  .def(
	    "RemoveValue",
	    +[](lua::State *l, T &p, int32_t idx) {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::Array>();
		    if(!a || idx < 0)
			    return;
		    a->RemoveValue(idx);
	    })
	  .def(
	    "RemoveValue",
	    +[](lua::State *l, T &p, const std::string &key) {
		    TPropertyWrapper tmp = static_cast<TPropertyWrapper>(p);
		    auto *el = tmp.template GetValuePtr<udm::Element>();
		    if(!el)
			    return;
		    auto it = el->children.find(key);
		    if(it != el->children.end())
			    el->children.erase(it);
	    })
	  .def(
	    "SetValue", +[](lua::State *l, T &el, const std::string &key, udm::Type type, luabind::object o) { set_property_value(l, static_cast<TPropertyWrapper>(el)[key], type, o, 4); })
	  .def(
	    "SetValue", +[](lua::State *l, T &el, int32_t idx, udm::Type type, luabind::object o) { set_property_value(l, static_cast<TPropertyWrapper>(el)[idx], type, o, 4); })
	  .def(
	    "SetValue",
	    +[](lua::State *l, T &el, const std::string &key, udm::Type type) {
		    if(type != udm::Type::Nil)
			    Lua::Error(l, "Omitting value only allowed for nil type!");
		    set_property_value(l, static_cast<TPropertyWrapper>(el)[key], type, luabind::object {}, 4, true);
	    })
	  .def(
	    "SetValue", +[](lua::State *l, T &el, const std::string &key, luabind::object o) { set_property_value(l, static_cast<TPropertyWrapper>(el)[key], o); })
	  .def(
	    "HasValue", +[](lua::State *l, T &p, const std::string &key) -> bool { return static_cast<bool>(static_cast<TPropertyWrapper>(p)[key]); })
	  .def(
	    "GetValue", +[](lua::State *l, T &p, const std::string &key) -> luabind::object { return get_property_value(l, static_cast<TPropertyWrapper>(p)[key]); })
	  .def(
	    "GetValue", +[](lua::State *l, T &p, const std::string &key, udm::Type type) -> luabind::object { return get_property_value(l, static_cast<TPropertyWrapper>(p)[key], type); })
	  .def(
	    "GetValue", +[](lua::State *l, T &p) -> luabind::object { return get_property_value(l, static_cast<TPropertyWrapper>(p)); })
	  .def(
	    "GetValue", +[](lua::State *l, T &p, udm::Type type) -> luabind::object { return get_property_value(l, static_cast<TPropertyWrapper>(p), type); })
	  .def(
	    "GetArrayValueDirect", +[](lua::State *l, T &p, int32_t idx) -> luabind::object { return get_property_value(l, static_cast<TPropertyWrapper>(p), idx); })
	  .def(
	    "GetType",
	    +[](lua::State *l, T &prop) -> udm::Type {
		    if constexpr(std::is_same_v<T, udm::Element>)
			    return udm::Type::Element;
		    else
			    return static_cast<TPropertyWrapper>(prop).GetType();
	    })
	  .def("Clear", &clear<T>)
	  .def(
	    "Merge", +[](lua::State *l, T &prop, udm::PropertyWrapper &propOther) { static_cast<TPropertyWrapper>(prop).Merge(propOther); })
	  .def(
	    "Merge", +[](lua::State *l, T &prop, udm::PropertyWrapper &propOther, udm::MergeFlags mergeFlags) { static_cast<TPropertyWrapper>(prop).Merge(propOther, mergeFlags); })
	  .def(
	    "Copy",
	    +[](lua::State *l, T &prop) -> std::shared_ptr<udm::Property> {
		    auto p = static_cast<TPropertyWrapper>(prop).prop;
		    if(!p)
			    return nullptr;
		    return pragma::util::make_shared<udm::Property>(*p);
	    })
	  .def(
	    "ToAscii",
	    +[](lua::State *l, T &prop) -> luabind::object {
		    auto *el = static_cast<TPropertyWrapper>(prop).template GetValuePtr<udm::Element>();
		    if(!el)
			    return {};
		    std::stringstream ss;
		    el->ToAscii(udm::AsciiSaveFlags::Default | udm::AsciiSaveFlags::DontCompressLz4Arrays, ss);
		    return luabind::object {l, ss.str()};
	    })
	  .def(
	    "ToAscii",
	    +[](lua::State *l, T &prop, udm::AsciiSaveFlags saveFlags) -> luabind::object {
		    auto *el = static_cast<TPropertyWrapper>(prop).template GetValuePtr<udm::Element>();
		    if(!el)
			    return {};
		    std::stringstream ss;
		    el->ToAscii(saveFlags, ss);
		    return luabind::object {l, ss.str()};
	    })
	  .def(
	    "ToTable",
	    +[](lua::State *l, T &v) -> Lua::tb<void> {
		    auto prop = static_cast<TPropertyWrapper>(v);
		    auto o = Lua::udm::udm_to_value(l, prop);
		    if(static_cast<Lua::Type>(luabind::type(o)) == Lua::Type::Table)
			    return o;
		    return luabind::newtable(l);
	    })
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, bool>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, float>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Vector2>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Vector3>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Vector4>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Quat>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, EulerAngles>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, pragma::math::Transform>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, pragma::math::ScaledTransform>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Mat4>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, Mat3x4>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, std::string>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, udm::Srgba>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, udm::HdrColor>)
	  .def("Set", &prop_set_basic_type_indexed<TPropertyWrapper, T, udm::Reference>)

	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, bool>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, float>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Vector2>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Vector3>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Vector4>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Quat>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, EulerAngles>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, pragma::math::Transform>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, pragma::math::ScaledTransform>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Mat4>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, Mat3x4>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, std::string>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, udm::Srgba>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, udm::HdrColor>)
	  .def("Set", &prop_set_basic_type<TPropertyWrapper, T, udm::Reference>)
	  .def(
	    "Set",
	    +[](lua::State *l, T &p, int32_t idx, pragma::util::DataStream &ds) {
		    auto offset = ds->GetOffset();
		    ds->SetOffset(0);
		    std::vector<uint8_t> data {};
		    data.resize(ds->GetInternalSize());
		    memcpy(data.data(), ds->GetData(), data.size());
		    static_cast<TPropertyWrapper>(p)[idx] = udm::Blob {std::move(data)};
		    ds->SetOffset(offset);
	    })
	  .def(
	    "Set",
	    +[](lua::State *l, T &p, const std::string &key, pragma::util::DataStream &ds) {
		    auto offset = ds->GetOffset();
		    ds->SetOffset(0);
		    std::vector<uint8_t> data {};
		    data.resize(ds->GetInternalSize());
		    memcpy(data.data(), ds->GetData(), data.size());
		    static_cast<TPropertyWrapper>(p)[key] = udm::Blob {std::move(data)};
		    ds->SetOffset(offset);
	    })
	  .def(
	    "AddArray", +[](lua::State *l, T &p, const std::string &name, uint32_t size) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).AddArray(name, size, udm::Type::Element); })
	  .def(
	    "AddArray", +[](lua::State *l, T &p, const std::string &name, uint32_t size, udm::Type type) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).AddArray(name, size, type); })
	  .def(
	    "AddArray", +[](lua::State *l, T &p, const std::string &name, uint32_t size, udm::Type type, udm::ArrayType arrayType) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).AddArray(name, size, type, arrayType); })
	  .def(
	    "GetFromPath", +[](lua::State *l, T &p, const std::string &path) -> udm::LinkedPropertyWrapper { return static_cast<TPropertyWrapper>(p).GetFromPath(path); })
	  .def(
	    "ClearUncompressedMemory",
	    +[](lua::State *l, T &p) {
		    auto *a = static_cast<TPropertyWrapper>(p).template GetValuePtr<udm::ArrayLz4>();
		    if(!a)
			    return;
		    a->ClearUncompressedMemory();
	    })
	  .def("IsValid", +[](lua::State *l, T &prop) -> bool { return static_cast<bool>(static_cast<TPropertyWrapper>(prop)); });
}

static void debug_print_wrapper(const udm::PropertyWrapper &lp, const std::string &t)
{
	std::cout << t << "arrayIndex: " << lp.arrayIndex << std::endl;
	std::cout << t << "prop: ";
	if(!lp.prop)
		std::cout << "NULL";
	else
		std::cout << udm::enum_to_string(lp.prop->type);
	std::cout << std::endl;
	auto *linked = lp.GetLinked();
	if(linked) {
		std::cout << t << "propName: " << linked->propName << std::endl;
		std::cout << "prev: ";
		if(!linked->prev)
			std::cout << "NULL" << std::endl;
		else {
			std::cout << std::endl;
			debug_print_wrapper(*linked->prev, t + "\t");
		}
	}
}
static void debug_print_wrapper(const udm::PropertyWrapper &lp) { debug_print_wrapper(lp, ""); }

static std::ostream &operator<<(std::ostream &os, const udm::Element &el) { return os << "UDMElement[" << el.fromProperty << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::Reference &ref) { return os << "UDMReference[" << ref.path << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::Data &data) { return os << "UDMData[" << data.GetAssetType() << "][" << data.GetAssetVersion() << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::PropertyWrapper &prop)
{
	auto valid = static_cast<bool>(prop);
	return os << "UDMPropertyWrapper[" << ::udm::enum_type_to_ascii(prop.GetType()) << "][" << (valid ? "valid" : "invalid") << "]";
}

static std::ostream &operator<<(std::ostream &os, const udm::LinkedPropertyWrapper &prop)
{
	auto valid = static_cast<bool>(prop);
	return os << "UDMLinkedPropertyWrapper[" << ::udm::enum_type_to_ascii(prop.GetType()) << "][" << (valid ? "valid" : "invalid") << "]";
}

static std::ostream &operator<<(std::ostream &os, const udm::AssetData &assetData) { return os << "UDMAssetData[" << assetData.GetAssetType() << "][" << assetData.GetAssetVersion() << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::Array &a) { return os << "UDMArray[" << ::udm::enum_type_to_ascii(a.GetValueType()) << "][" << a.GetSize() << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::ArrayLz4 &a) { return os << "UDMArrayLz4[" << ::udm::enum_type_to_ascii(a.GetValueType()) << "][" << a.GetSize() << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::Property &prop) { return os << "UDMProperty[" << ::udm::enum_type_to_ascii(prop.type) << "]"; }

static std::ostream &operator<<(std::ostream &os, const udm::Srgba &srgba) { return os << srgba[0] << " " << srgba[1] << " " << srgba[2] << " " << srgba[3]; }

static std::ostream &operator<<(std::ostream &os, const udm::HdrColor &hdr) { return os << hdr[0] << " " << hdr[1] << " " << hdr[2] << " " << hdr[3]; }

namespace Lua::udm {
	void register_types(Interface &lua, luabind::module_ &modUdm);
};
#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(udm, udm::Element);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(udm, udm::Reference);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(udm, udm::Data);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(udm, udm::Array);
#endif
void Lua::udm::register_types(Interface &lua, luabind::module_ &modUdm)
{
	auto cdEl = luabind::class_<::udm::Element>("Element");
	cdEl.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::Element, ::udm::PropertyWrapper &>(cdEl);
	modUdm[cdEl];

	auto cdRef = luabind::class_<::udm::Reference>("Reference");
	cdRef.def(luabind::constructor<>());
	cdRef.def(luabind::constructor<const std::string &>());
	cdRef.def(luabind::tostring(luabind::self));
	cdRef.def("GetProperty", +[](lua::State *l, ::udm::Reference &ref) -> ::udm::PropertyWrapper { return ref.property ? ::udm::PropertyWrapper {*ref.property} : ::udm::PropertyWrapper {}; });
	modUdm[cdRef];

	auto cdStructDescription = luabind::class_<::udm::StructDescription>("StructDescription");
	cdStructDescription.def(luabind::constructor<>());
	cdStructDescription.def("GetTypes", +[](lua::State *l, ::udm::StructDescription &udmData) -> luabind::object { return Lua::vector_to_table<::udm::Type>(l, udmData.types); });
	cdStructDescription.def("GetNames", +[](lua::State *l, ::udm::StructDescription &udmData) -> luabind::object { return Lua::vector_to_table<std::string>(l, udmData.names); });
	cdStructDescription.def("GetMemberCount", +[](lua::State *l, ::udm::StructDescription &udmData) -> uint32_t { return udmData.GetMemberCount(); });
	modUdm[cdStructDescription];

	auto cdStruct = luabind::class_<::udm::Struct>("Struct");
	cdStruct.def(luabind::constructor<>());
	cdStruct.def(luabind::constructor<const ::udm::StructDescription &>());
	cdStruct.def("GetDescription", +[](lua::State *l, ::udm::Struct &strct) -> ::udm::StructDescription * { return &strct.description; });
	cdStruct.def(
	  "GetValue", +[](lua::State *l, ::udm::Struct &strct) -> luabind::object {
		  auto t = luabind::newtable(l);
		  auto n = strct.description.types.size();
		  auto *ptr = strct.data.data();
		  auto vs = [l, &t, &ptr](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(std::is_same_v<T, ::udm::Nil>)
				  return luabind::object {};
			  else {

				  auto &v = *reinterpret_cast<T *>(ptr);
				  if constexpr(std::is_same_v<T, ::udm::Half>)
					  return luabind::object {l, static_cast<float>(v)};
				  return luabind::object {l, v};
			  }
		  };
		  for(auto i = decltype(n) {0u}; i < n; ++i) {
			  t[i + 1] = ::udm::visit<false, true, false>(strct.description.types[i], vs);
			  ptr += ::udm::size_of(strct.description.types[i]);
		  }
		  return t;
	  });
	modUdm[cdStruct];

	auto cdSrgba = luabind::class_<::udm::Srgba>("Srgba");
	cdSrgba.def(luabind::constructor<>());
	cdSrgba.def(
	  "__tostring", +[](const ::udm::Srgba &value) -> std::string {
		  std::stringstream ss;
		  ss << value;
		  return ss.str();
	  });
	cdSrgba.property("r", +[](lua::State *l, ::udm::Srgba &srgba) -> uint8_t { return srgba[0]; }, +[](lua::State *l, ::udm::Srgba &srgba, uint8_t r) { srgba[0] = r; });
	cdSrgba.property("g", +[](lua::State *l, ::udm::Srgba &srgba) -> uint8_t { return srgba[1]; }, +[](lua::State *l, ::udm::Srgba &srgba, uint8_t g) { srgba[1] = g; });
	cdSrgba.property("b", +[](lua::State *l, ::udm::Srgba &srgba) -> uint8_t { return srgba[2]; }, +[](lua::State *l, ::udm::Srgba &srgba, uint8_t b) { srgba[2] = b; });
	cdSrgba.property("a", +[](lua::State *l, ::udm::Srgba &srgba) -> uint8_t { return srgba[3]; }, +[](lua::State *l, ::udm::Srgba &srgba, uint8_t a) { srgba[3] = a; });
	modUdm[cdSrgba];

	auto cdHdr = luabind::class_<::udm::HdrColor>("HdrColor");
	cdHdr.def(luabind::constructor<>());
	cdHdr.def(
	  "__tostring", +[](const ::udm::HdrColor &value) -> std::string {
		  std::stringstream ss;
		  ss << value;
		  return ss.str();
	  });
	cdHdr.property("r", +[](lua::State *l, ::udm::HdrColor &srgba) -> uint8_t { return srgba[0]; }, +[](lua::State *l, ::udm::HdrColor &srgba, uint8_t r) { srgba[0] = r; });
	cdHdr.property("g", +[](lua::State *l, ::udm::HdrColor &srgba) -> uint8_t { return srgba[1]; }, +[](lua::State *l, ::udm::HdrColor &srgba, uint8_t g) { srgba[1] = g; });
	cdHdr.property("b", +[](lua::State *l, ::udm::HdrColor &srgba) -> uint8_t { return srgba[2]; }, +[](lua::State *l, ::udm::HdrColor &srgba, uint8_t b) { srgba[2] = b; });
	cdHdr.property("a", +[](lua::State *l, ::udm::HdrColor &srgba) -> uint8_t { return srgba[3]; }, +[](lua::State *l, ::udm::HdrColor &srgba, uint8_t a) { srgba[3] = a; });
	modUdm[cdHdr];

	auto cdData = luabind::class_<::udm::Data>("Data");
	cdData.def(luabind::tostring(luabind::self));
	cdData.def(
	  "Save", +[](lua::State *l, ::udm::Data &udmData, const std::string &fileName) {
		  auto fname = fileName;
		  if(file::validate_write_operation(l, fname) == false) {
			  PushBool(l, false);
			  PushString(l, "Invalid write location!");
			  return;
		  }
		  try {
			  udmData.Save(fname);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "SaveAscii", +[](lua::State *l, ::udm::Data &udmData, const std::string &fileName) {
		  auto fname = fileName;
		  if(file::validate_write_operation(l, fname) == false) {
			  PushBool(l, false);
			  PushString(l, "Invalid write location!");
			  return;
		  }
		  try {
			  udmData.SaveAscii(fname);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "SaveAscii", +[](lua::State *l, ::udm::Data &udmData, const std::string &fileName, ::udm::AsciiSaveFlags flags) {
		  auto fname = fileName;
		  if(file::validate_write_operation(l, fname) == false) {
			  PushBool(l, false);
			  PushString(l, "Invalid write location!");
			  return;
		  }
		  try {
			  udmData.SaveAscii(fname, flags);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "Save", +[](lua::State *l, ::udm::Data &udmData, LFile &f) {
		  auto fptr = f.GetHandle();
		  if(fptr == nullptr)
			  return;
		  try {
			  udmData.Save(*fptr);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "SaveAscii", +[](lua::State *l, ::udm::Data &udmData, LFile &f) {
		  auto fptr = f.GetHandle();
		  if(fptr == nullptr)
			  return;
		  try {
			  udmData.SaveAscii(*fptr);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "SaveAscii", +[](lua::State *l, ::udm::Data &udmData, LFile &f, ::udm::AsciiSaveFlags flags) {
		  auto fptr = f.GetHandle();
		  if(fptr == nullptr)
			  return;
		  try {
			  udmData.SaveAscii(*fptr, flags);
			  PushBool(l, true);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def(
	  "ToAscii", +[](lua::State *l, ::udm::Data &udmData) -> std::string {
		  std::stringstream ss;
		  udmData.ToAscii(ss);
		  return ss.str();
	  });
	cdData.def("GetAssetData", +[](lua::State *l, ::udm::Data &udmData) -> ::udm::AssetData { return udmData.GetAssetData(); });
	cdData.def(
	  "LoadProperty", +[](lua::State *l, ::udm::Data &udmData, const std::string &path) {
		  try {
			  auto prop = udmData.LoadProperty(path);
			  Push(l, prop);
		  }
		  catch(const ::udm::Exception &e) {
			  PushBool(l, false);
			  PushString(l, e.what());
		  }
	  });
	cdData.def("GetRootElement", +[](lua::State *l, ::udm::Data &udmData) -> ::udm::Element * { return &udmData.GetRootElement(); });
	cdData.def("Get", +[](lua::State *l, ::udm::Data &udmData, const std::string &key) -> ::udm::LinkedPropertyWrapper { return udmData.GetRootElement()[key]; });
	cdData.def("ResolveReferences", +[](lua::State *l, ::udm::Data &udmData) { udmData.ResolveReferences(); });
	modUdm[cdData];

	auto cdPropWrap = luabind::class_<::udm::PropertyWrapper>("PropertyWrapper");
	cdPropWrap.def(luabind::constructor<>());
	cdPropWrap.def(luabind::tostring(luabind::self));
	static std::optional<LuaUdmArrayIterator> g_it {}; // HACK: This is a workaround for a bug in luabind, which causes errors when compiled with gcc.
	cdPropWrap.def(
	  "It",
	  +[](lua::State *l, ::udm::PropertyWrapper &p) -> LuaUdmArrayIterator & {
		  g_it = LuaUdmArrayIterator {p};
		  return *g_it;
	  },
	  luabind::return_stl_iterator {});
	cdPropWrap.def("DebugPrint", static_cast<void (*)(const ::udm::PropertyWrapper &)>(&debug_print_wrapper));
	register_property_methods<::udm::PropertyWrapper, ::udm::PropertyWrapper &>(cdPropWrap);
	modUdm[cdPropWrap];

	auto cdLinkedPropWrap = luabind::class_<::udm::LinkedPropertyWrapper, ::udm::PropertyWrapper>("LinkedPropertyWrapper");
	cdLinkedPropWrap.def(luabind::tostring(luabind::self));
	cdLinkedPropWrap.def(luabind::constructor<>());
	cdLinkedPropWrap.def("GetAssetData", +[](lua::State *l, ::udm::LinkedPropertyWrapper &prop) -> ::udm::AssetData { return ::udm::AssetData {prop}; });
	cdLinkedPropWrap.def("GetPath", +[](lua::State *l, ::udm::LinkedPropertyWrapper &prop) -> std::string { return prop.GetPath(); });
	cdLinkedPropWrap.def("ClaimOwnership", +[](lua::State *l, ::udm::LinkedPropertyWrapper &prop) -> ::udm::PProperty { return prop.ClaimOwnership(); });
	modUdm[cdLinkedPropWrap];
	pragma::LuaCore::define_custom_constructor<::udm::LinkedPropertyWrapper, +[](::udm::Property &prop) -> ::udm::LinkedPropertyWrapper { return ::udm::LinkedPropertyWrapper {prop}; }, ::udm::Property &>(lua.GetState());
	pragma::LuaCore::define_custom_constructor<::udm::LinkedPropertyWrapper, +[](::udm::LinkedPropertyWrapper &prop) -> ::udm::LinkedPropertyWrapper { return prop; }, ::udm::LinkedPropertyWrapper &>(lua.GetState());

	auto cdAssetData = luabind::class_<::udm::AssetData, ::udm::LinkedPropertyWrapper, ::udm::PropertyWrapper>("AssetData");
	cdAssetData.def(luabind::tostring(luabind::self));
	cdAssetData.def("GetAssetType", &::udm::AssetData::GetAssetType);
	cdAssetData.def("GetAssetVersion", &::udm::AssetData::GetAssetVersion);
	cdAssetData.def("SetAssetType", &::udm::AssetData::SetAssetType);
	cdAssetData.def("SetAssetVersion", &::udm::AssetData::SetAssetVersion);
	cdAssetData.def("GetData", +[](lua::State *l, ::udm::AssetData &a) -> ::udm::LinkedPropertyWrapper { return a.GetData(); });
	cdAssetData.def("SetData", +[](lua::State *l, ::udm::AssetData &a, ::udm::PProperty &prop) { a.GetData() = prop; });
	modUdm[cdAssetData];
	pragma::LuaCore::define_custom_constructor<::udm::AssetData,
	  +[](::udm::LinkedPropertyWrapper &prop, const std::string &identifier, ::udm::Version version) -> ::udm::AssetData {
		  ::udm::AssetData assetData {prop};
		  assetData.SetAssetType(identifier);
		  assetData.SetAssetVersion(version);
		  return assetData;
	  },
	  ::udm::LinkedPropertyWrapper &, const std::string &, ::udm::Version>(lua.GetState());

	auto cdArray = luabind::class_<::udm::Array>("Array");
	cdArray.def(luabind::tostring(luabind::self));
	cdArray.def("GetArrayType", &::udm::Array::GetArrayType);
	cdArray.def(
	  "ToTable", +[](lua::State *l, ::udm::Array &a) -> tb<void> {
		  auto t = luabind::newtable(l);
		  for(uint32_t idx = 1; auto &val : a)
			  t[idx++] = udm_to_value(l, val);
		  return t;
	  });
	modUdm[cdArray];

	auto cdProp = luabind::class_<::udm::Property>("Property");
	cdProp.def(luabind::tostring(luabind::self));
	register_property_methods<::udm::Property, ::udm::PropertyWrapper>(cdProp);
	//cdProp.def("GetType",+[](lua::State *l,::udm::Property &prop) -> ::udm::Type {
	//	return prop.type;
	//});
	cdProp.def(
	  "ToAscii", +[](lua::State *l, ::udm::Property &prop, const std::string &propName, const std::string &prefix) -> std::string {
		  std::stringstream ss;
		  prop.ToAscii(::udm::AsciiSaveFlags::Default, ss, propName, prefix);
		  return ss.str();
	  });
	cdProp.def(
	  "ToAscii", +[](lua::State *l, ::udm::Property &prop, const std::string &propName) -> std::string {
		  std::stringstream ss;
		  prop.ToAscii(::udm::AsciiSaveFlags::Default, ss, propName);
		  return ss.str();
	  });
	cdProp.def("Get", +[](lua::State *l, ::udm::Property &prop) -> ::udm::LinkedPropertyWrapper { return ::udm::LinkedPropertyWrapper {prop}; });
	modUdm[cdProp];

	// TODO: Register "Get" method as subscript operator for the Lua classes
	// Attempts below don't work
	/*auto *reg = luabind::detail::class_registry::get_registry(lua.GetState());
	auto *cls = reg->find_class(typeid(::udm::Property));
	lua::raw_get(lua.GetState(), Lua::RegistryIndex, cls->metatable_ref());
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
