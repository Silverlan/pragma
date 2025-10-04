// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "any"

#include "mathutil/umath.h"

#include "variant"

#include "cassert"

#include "sharedutils/util.h"

#include "pragma/lua/luaapi.h"

#include "mathutil/uvec.h"

#include <sharedutils/datastream.h>
#include <mathutil/color.h>

module pragma.shared;

import :util.any;

std::string util::variable_type_to_string(VarType type)
{
	switch(type) {
	case VarType::Invalid:
		return "Invalid";
	case VarType::Bool:
		return "Bool";
	case VarType::Double:
		return "Double";
	case VarType::Float:
		return "Float";
	case VarType::Int8:
		return "Int8";
	case VarType::Int16:
		return "Int16";
	case VarType::Int32:
		return "Int32";
	case VarType::Int64:
		return "Int64";
	case VarType::LongDouble:
		return "LongDouble";
	case VarType::String:
		return "String";
	case VarType::UInt8:
		return "UInt8";
	case VarType::UInt16:
		return "UInt16";
	case VarType::UInt32:
		return "UInt32";
	case VarType::UInt64:
		return "UInt64";
	case VarType::EulerAngles:
		return "EulerAngles";
	case VarType::Color:
		return "Color";
	case VarType::Vector:
		return "Vector";
	case VarType::Vector2:
		return "Vector2";
	case VarType::Vector4:
		return "Vector4";
	case VarType::Entity:
		return "Entity";
	case VarType::Quaternion:
		return "Quaternion";
	case VarType::Transform:
		return "Transform";
	case VarType::ScaledTransform:
		return "ScaledTransform";
	case VarType::Count:
		return "Count";
	}
	static_assert(umath::to_integral(VarType::Count) == 23);
	return "Unknown";
}

struct IAnyHandler {
	virtual std::any GetValue(lua_State *l, int32_t idx) const = 0;
	virtual void Push(lua_State *l, const std::any &value) const = 0;
	virtual void PushNewProperty(lua_State *l, const std::any &value) const = 0;

	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const = 0;
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const = 0;

	virtual void Write(DataStream &ds, const std::any &value, uint32_t *pos = nullptr) const = 0;
	virtual void Write(NetPacket &ds, const std::any &value, uint32_t *pos = nullptr) const = 0;
	virtual void Read(Game &game, DataStream &ds, std::any &outValue) const = 0;
	virtual void Read(NetPacket &ds, std::any &outValue) const = 0;
	template<typename T>
	const T &Get(const std::any &value) const
	{
		return std::any_cast<const T &>(value);
	}
};

template<class T, class TProperty, class TLuaProperty, bool (*TIs)(lua_State *, int32_t), T (*TCheck)(lua_State *, int32_t), void (*TPush)(lua_State *, T)>
struct TGenericHandler : public IAnyHandler {
	virtual std::any GetValue(lua_State *l, int32_t idx) const override
	{
		if(TIs(l, idx) == false)
			return T {};
		return {static_cast<T>(TCheck(l, idx))};
	}
	virtual void Push(lua_State *l, const std::any &value) const override { TPush(l, std::any_cast<T>(value)); }
	virtual void PushNewProperty(lua_State *l, const std::any &value) const override { Lua::Property::push(l, *TProperty::Create(Get<T>(value))); }
	virtual void Write(DataStream &ds, const std::any &value, uint32_t *pos = nullptr) const override { ds->Write<T>(Get<T>(value), pos); }
	virtual void Write(NetPacket &ds, const std::any &value, uint32_t *pos = nullptr) const override { ds->Write<T>(Get<T>(value), pos); }
	virtual void Read(Game &game, DataStream &ds, std::any &outValue) const override { outValue = ds->Read<T>(); }
	virtual void Read(NetPacket &ds, std::any &outValue) const override { outValue = ds->Read<T>(); }
};

template<class T>
T check_user_class(lua_State *l, int32_t index)
{
	return Lua::Check<T>(l, index);
}

template<class T>
void push_user_class(lua_State *l, T value)
{
	Lua::Push<T>(l, value);
}

template<class T, class TProperty, class TLuaProperty, bool (*TIs)(lua_State *, int32_t), T (*TCheck)(lua_State *, int32_t), void (*TPush)(lua_State *, T)>
struct TGenericBasePropertyUserClassHandler : public TGenericHandler<T, TProperty, TLuaProperty, TIs, TCheck, TPush> {
	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const override
	{
		if(Lua::IsType<TLuaProperty>(l, indexProperty) == false)
			return;
		*Lua::Check<TLuaProperty>(l, indexProperty) = std::any_cast<T>(value);
	}
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const override
	{
		if(Lua::IsType<TLuaProperty>(l, indexProperty) == false)
			return T {};
		return static_cast<T>(Lua::Check<TLuaProperty>(l, indexProperty)->GetValue());
	}
};

template<class T, class TProperty, class TLuaProperty, T (*TCheck)(lua_State *, int32_t), void (*TPush)(lua_State *, T)>
struct TGenericPropertyUserClassHandler : public TGenericBasePropertyUserClassHandler<T, TProperty, TLuaProperty, Lua::IsType<T>, TCheck, TPush> {};

// Note: This would be 'prettier' than creating a class, but causes compiler errors for some unknown reason
//template<class T,class TProperty,class TLuaProperty>
//	using TGenericUserClassHandler = TGenericPropertyUserClassHandler<T,TProperty,TLuaProperty,check_user_class<T>,push_user_class<T>>;
template<class T, class TProperty, class TLuaProperty>
struct TGenericUserClassHandler : public TGenericPropertyUserClassHandler<T, TProperty, TLuaProperty, check_user_class<T>, push_user_class<T>> {};

template<class T, class TProperty, class TLuaProperty>
struct TGenericIntegerHandler : public TGenericHandler<T, TProperty, TLuaProperty, Lua::IsNumber, Lua::CheckInt, Lua::PushInt> {
	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const override
	{
		if(Lua::IsType<LGenericIntPropertyWrapper>(l, indexProperty) == false)
			return;
		Lua::Check<LGenericIntPropertyWrapper>(l, indexProperty) = std::any_cast<T>(value);
	}
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const override
	{
		if(Lua::IsType<LGenericIntPropertyWrapper>(l, indexProperty) == false)
			return T {};
		return static_cast<T>(Lua::Check<LGenericIntPropertyWrapper>(l, indexProperty)->GetValue());
	}
};

template<class T, class TProperty, class TLuaProperty>
struct TGenericFloatHandler : public TGenericHandler<T, TProperty, TLuaProperty, Lua::IsNumber, Lua::CheckNumber, Lua::PushNumber> {
	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const override
	{
		if(Lua::IsType<LGenericIntPropertyWrapper>(l, indexProperty) == false)
			return;
		Lua::Check<LGenericFloatPropertyWrapper>(l, indexProperty) = std::any_cast<T>(value);
	}
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const override
	{
		if(Lua::IsType<LGenericIntPropertyWrapper>(l, indexProperty) == false)
			return T {};
		return static_cast<T>(Lua::Check<LGenericFloatPropertyWrapper>(l, indexProperty)->GetValue());
	}
};

static TGenericBasePropertyUserClassHandler<bool, util::BoolProperty, LBoolProperty, Lua::IsBool, Lua::CheckBool, Lua::PushBool> s_boolHandler;

static TGenericFloatHandler<double, util::DoubleProperty, LGenericFloatPropertyWrapper> s_doubleHandler;
static TGenericFloatHandler<float, util::FloatProperty, LGenericFloatPropertyWrapper> s_floatHandler;
static TGenericIntegerHandler<int8_t, util::Int8Property, LGenericIntPropertyWrapper> s_int8Handler;
static TGenericIntegerHandler<int16_t, util::Int16Property, LGenericIntPropertyWrapper> s_int16Handler;
static TGenericIntegerHandler<int32_t, util::Int32Property, LGenericIntPropertyWrapper> s_int32Handler;
static TGenericIntegerHandler<int64_t, util::Int64Property, LGenericIntPropertyWrapper> s_int64Handler;
static TGenericIntegerHandler<long double, util::LongDoubleProperty, LGenericFloatPropertyWrapper> s_longDoubleHandler;

static std::string property_check_string(lua_State *l, int32_t idx) { return Lua::CheckString(l, idx); }

static void property_push_string(lua_State *l, std::string str) { Lua::PushString(l, str); }

static TGenericPropertyUserClassHandler<std::string, util::StringProperty, LStringProperty, property_check_string, property_push_string> s_stringHandler;

static TGenericIntegerHandler<uint8_t, util::UInt8Property, LGenericIntPropertyWrapper> s_uint8Handler;
static TGenericIntegerHandler<uint16_t, util::UInt16Property, LGenericIntPropertyWrapper> s_uint16Handler;
static TGenericIntegerHandler<uint32_t, util::UInt32Property, LGenericIntPropertyWrapper> s_uint32Handler;
static TGenericIntegerHandler<uint64_t, util::UInt64Property, LGenericIntPropertyWrapper> s_uint64Handler;

static TGenericUserClassHandler<EulerAngles, util::EulerAnglesProperty, LEulerAnglesProperty> s_eulerAnglesHandler;
static TGenericUserClassHandler<Color, util::ColorProperty, LColorProperty> s_colorHandler;
static TGenericUserClassHandler<Vector3, util::Vector3Property, LVector3Property> s_vector3Handler;
static TGenericUserClassHandler<Vector2, util::Vector2Property, LVector2Property> s_vector2Handler;
static TGenericUserClassHandler<Vector4, util::Vector4Property, LVector4Property> s_vector4Handler;
static TGenericUserClassHandler<Quat, util::QuatProperty, LQuatProperty> s_quatHandler;
static TGenericUserClassHandler<umath::Transform, util::TransformProperty, LTransformProperty> s_transformHandler;
static TGenericUserClassHandler<umath::ScaledTransform, util::ScaledTransformProperty, LScaledTransformProperty> s_scaledTransformHandler;

struct EntityHandler : public IAnyHandler {
	virtual std::any GetValue(lua_State *l, int32_t idx) const override
	{
		if(Lua::IsType<pragma::EntityURef>(l, idx) == false)
			return pragma::EntityURef {};
		return Lua::Check<pragma::EntityURef>(l, idx);
	}
	virtual void Push(lua_State *l, const std::any &value) const override { Lua::Push(l, Get<pragma::EntityURef>(value)); }
	virtual void PushNewProperty(lua_State *l, const std::any &value) const override
	{
		// Lua::Property::push(l,*pragma::EntityProperty::Create(Get<EntityHandle>(value)));
	}
	virtual void Write(DataStream &ds, const std::any &value, uint32_t *pos = nullptr) const override
	{
		// auto hEnt = Get<EntityHandle>(value);
		// auto idx = hEnt.valid() ? hEnt->GetIndex() : std::numeric_limits<uint32_t>::max();
		// ds->Write<uint32_t>(idx);
	}
	virtual void Write(NetPacket &ds, const std::any &value, uint32_t *pos = nullptr) const override
	{
		auto &ref = Get<pragma::EntityURef>(value);
		auto *identifier = ref.GetIdentifier();
		if(!identifier) {
			ds->Write<uint8_t>(static_cast<uint8_t>(0));
			return;
		}
		std::visit(
		  [&ds](auto &val) {
			  using T = util::base_type<decltype(val)>;
			  if constexpr(std::is_same_v<T, util::Uuid>) {
				  ds->Write<uint8_t>(static_cast<uint8_t>(1));
				  ds->Write(reinterpret_cast<const uint8_t *>(&val[0]), sizeof(util::Uuid));
			  }
			  else {
				  ds->Write<uint8_t>(static_cast<uint8_t>(2));
				  ds->WriteString(val);
			  }
		  },
		  *identifier);
	}
	virtual void Read(Game &game, DataStream &ds, std::any &outValue) const override
	{
		outValue = pragma::EntityURef {};
		// auto idx = ds->Read<uint32_t>();
		// auto *ent = game.GetEntity(idx);
		// outValue = (ent != nullptr) ? ent->GetHandle() : EntityHandle{};
	}
	virtual void Read(NetPacket &ds, std::any &outValue) const override
	{
		auto type = ds->Read<uint8_t>();
		switch(type) {
		case 0:
			outValue = pragma::EntityURef {};
			return;
		case 1:
			outValue = pragma::EntityURef {ds->Read<util::Uuid>()};
			return;
		case 2:
			outValue = pragma::EntityURef {ds->ReadString()};
			return;
		}
	}
	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const override
	{
		// if(Lua::IsType<LEntityProperty>(l,indexProperty) == false)
		// 	return;
		// *Lua::Check<LEntityProperty>(l,indexProperty) = std::any_cast<EntityHandle>(value);
	}
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const override
	{
		return pragma::EntityURef {};
		// if(Lua::IsType<LEntityProperty>(l,indexProperty) == false)
		// 	return EntityHandle{};
		// return static_cast<EntityHandle>(Lua::Check<LEntityProperty>(l,indexProperty)->GetValue());
	}
} static s_entityHandler;

struct NilHandler : public IAnyHandler {
	virtual std::any GetValue(lua_State *l, int32_t idx) const override { return {}; }
	virtual void Push(lua_State *l, const std::any &value) const override { Lua::PushNil(l); }
	virtual void PushNewProperty(lua_State *l, const std::any &value) const override { Lua::PushNil(l); }
	virtual void SetPropertyValue(lua_State *l, int32_t indexProperty, const std::any &value) const override {}
	virtual std::any GetPropertyValue(lua_State *l, int32_t indexProperty) const override { return {}; }
	virtual void Write(DataStream &ds, const std::any &value, uint32_t *pos = nullptr) const override {}
	virtual void Write(NetPacket &ds, const std::any &value, uint32_t *pos = nullptr) const override {}
	virtual void Read(Game &game, DataStream &ds, std::any &outValue) const override { outValue = {}; }
	virtual void Read(NetPacket &ds, std::any &outValue) const override { outValue = {}; }
} static s_nilHandler;

// If this assert fails, it means a new variable type as been added to the enum list which hasn't been included in this list yet
static_assert(umath::to_integral(util::VarType::Count) == 23u);
static constexpr const IAnyHandler &get_any_handler(util::VarType varType)
{
	switch(varType) {
	case util::VarType::Bool:
		return s_boolHandler;
	case util::VarType::Double:
		return s_doubleHandler;
	case util::VarType::Float:
		return s_floatHandler;
	case util::VarType::Int8:
		return s_int8Handler;
	case util::VarType::Int16:
		return s_int16Handler;
	case util::VarType::Int32:
		return s_int32Handler;
	case util::VarType::Int64:
		return s_int64Handler;
	case util::VarType::LongDouble:
		return s_longDoubleHandler;
	case util::VarType::String:
		return s_stringHandler;
	case util::VarType::UInt8:
		return s_uint8Handler;
	case util::VarType::UInt16:
		return s_uint16Handler;
	case util::VarType::UInt32:
		return s_uint32Handler;
	case util::VarType::UInt64:
		return s_uint64Handler;
	case util::VarType::EulerAngles:
		return s_eulerAnglesHandler;
	case util::VarType::Color:
		return s_colorHandler;
	case util::VarType::Vector:
		return s_vector3Handler;
	case util::VarType::Vector2:
		return s_vector2Handler;
	case util::VarType::Vector4:
		return s_vector4Handler;
	case util::VarType::Entity:
		return s_entityHandler;
	case util::VarType::Quaternion:
		return s_quatHandler;
	case util::VarType::Transform:
		return s_transformHandler;
	case util::VarType::ScaledTransform:
		return s_scaledTransformHandler;
	}
	return s_nilHandler;
}

std::any Lua::GetAnyValue(lua_State *l, ::util::VarType varType, int32_t idx) { return get_any_handler(varType).GetValue(l, idx); }
std::any Lua::GetAnyPropertyValue(lua_State *l, int32_t indexProperty, ::util::VarType varType) { return get_any_handler(varType).GetPropertyValue(l, indexProperty); }
void Lua::SetAnyPropertyValue(lua_State *l, int32_t indexProperty, ::util::VarType varType, const std::any &value) { get_any_handler(varType).SetPropertyValue(l, indexProperty, value); }
void Lua::PushAny(lua_State *l, ::util::VarType varType, const std::any &value) { get_any_handler(varType).Push(l, value); }
void Lua::PushNewAnyProperty(lua_State *l, ::util::VarType varType, const std::any &value) { get_any_handler(varType).PushNewProperty(l, value); }
void Lua::WriteAny(::DataStream &ds, ::util::VarType varType, const std::any &value, uint32_t *pos) { get_any_handler(varType).Write(ds, value, pos); }
void Lua::WriteAny(::NetPacket &ds, ::util::VarType varType, const std::any &value, uint32_t *pos) { get_any_handler(varType).Write(ds, value, pos); }
void Lua::ReadAny(Game &game, ::DataStream &ds, ::util::VarType varType, std::any &outValue) { get_any_handler(varType).Read(game, ds, outValue); }
void Lua::ReadAny(::NetPacket &ds, ::util::VarType varType, std::any &outValue) { get_any_handler(varType).Read(ds, outValue); }
