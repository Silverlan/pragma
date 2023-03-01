/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/classes/lproperty_generic.hpp"
#include "pragma/lua/lua_call.hpp"
#include <pragma/lua/luaapi.h>
#include <luainterface.hpp>

static float operator+(float f, const LGenericFloatPropertyWrapper &prop) { return f + prop.GetValue(); }
static float operator-(float f, const LGenericFloatPropertyWrapper &prop) { return f - prop.GetValue(); }
static float operator*(float f, const LGenericFloatPropertyWrapper &prop) { return f * prop.GetValue(); }
static float operator/(float f, const LGenericFloatPropertyWrapper &prop) { return f / prop.GetValue(); }
static bool operator==(float f, const LGenericFloatPropertyWrapper &prop) { return f == prop.GetValue(); }
static bool operator<(float f, const LGenericFloatPropertyWrapper &prop) { return f < prop.GetValue(); }
static bool operator<=(float f, const LGenericFloatPropertyWrapper &prop) { return f <= prop.GetValue(); }
static std::ostream &operator<<(std::ostream &str, const LGenericFloatPropertyWrapper &v) { return str << v.GetValue(); }

//////////

static float operator+(int32_t f, const LGenericIntPropertyWrapper &prop) { return f + prop.GetValue(); }
static float operator-(int32_t f, const LGenericIntPropertyWrapper &prop) { return f - prop.GetValue(); }
static float operator*(int32_t f, const LGenericIntPropertyWrapper &prop) { return f * prop.GetValue(); }
static float operator/(int32_t f, const LGenericIntPropertyWrapper &prop) { return f / prop.GetValue(); }
static bool operator==(int32_t f, const LGenericIntPropertyWrapper &prop) { return f == prop.GetValue(); }
static bool operator<(int32_t f, const LGenericIntPropertyWrapper &prop) { return f < prop.GetValue(); }
static bool operator<=(int32_t f, const LGenericIntPropertyWrapper &prop) { return f <= prop.GetValue(); }
static std::ostream &operator<<(std::ostream &str, const LGenericIntPropertyWrapper &v) { return str << v.GetValue(); }

//////////

static bool operator==(bool v, const LBoolProperty &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LBoolProperty &v) { return str << **v; }

//////////

static Color operator+(Color v, const LColorPropertyWrapper &prop) { return v + prop->GetValue(); }
static Color operator-(Color v, const LColorPropertyWrapper &prop) { return v - prop->GetValue(); }
static Color operator*(float v, const LColorPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(Color v, const LColorPropertyWrapper &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LColorPropertyWrapper &v) { return str << **v; }

//////////

static EulerAngles operator+(EulerAngles v, const LEulerAnglesPropertyWrapper &prop) { return v + prop->GetValue(); }
static EulerAngles operator-(EulerAngles v, const LEulerAnglesPropertyWrapper &prop) { return v - prop->GetValue(); }
static EulerAngles operator*(float v, const LEulerAnglesPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(EulerAngles v, const LEulerAnglesPropertyWrapper &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LEulerAnglesPropertyWrapper &v) { return str << **v; }

//////////

static umath::Transform operator*(umath::Transform v, const LTransformPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(umath::Transform v, const LTransformPropertyWrapper &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LTransformPropertyWrapper &v) { return str << **v; }

//////////

static umath::ScaledTransform operator*(umath::ScaledTransform v, const LScaledTransformPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(umath::ScaledTransform v, const LScaledTransformPropertyWrapper &prop) { return **prop == v; }
// static std::ostream &operator<<(std::ostream &str, const LScaledTransformPropertyWrapper &v) { return str << **v; }

//////////

static Quat operator*(const Quat &v, const LQuatProperty &prop) { return v * prop->GetValue(); }
static Quat operator*(float v, const LQuatProperty &prop) { return v * prop->GetValue(); }
static bool operator==(const Quat &v, const LQuatProperty &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LQuatProperty &v)
{
	auto &rot = **v;
	return str << rot.w << " " << rot.x << " " << rot.y << " " << rot.z;
}

//////////

static bool operator==(const std::string &v, const LStringProperty &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LStringProperty &v) { return str << **v; }

//////////

static std::ostream &operator<<(std::ostream &str, const LMatrix2Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[1][0] << " " << m[1][1];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix2x3Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[1][0] << " " << m[1][1] << " " << m[1][2];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix3x2Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[1][0] << " " << m[1][1] << " " << m[2][0] << " " << m[2][1];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix3Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[2][0] << " " << m[2][1] << " " << m[2][2];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix3x4Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << " " << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << " " << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix4x3Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[3][0] << " " << m[3][1] << " " << m[3][2];
}
static std::ostream &operator<<(std::ostream &str, const LMatrix4Property &prop)
{
	auto &m = **prop;
	return str << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << " " << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << " " << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << " " << m[3][0] << " " << m[3][1] << " " << m[3][2] << " "
	           << m[3][3];
}

static bool operator==(const Mat2 &v, const LMatrix2Property &prop) { return **prop == v; }
static bool operator==(const Mat2x3 &v, const LMatrix2x3Property &prop) { return **prop == v; }
static bool operator==(const Mat3x2 &v, const LMatrix3x2Property &prop) { return **prop == v; }
static bool operator==(const Mat3 &v, const LMatrix3Property &prop) { return **prop == v; }
static bool operator==(const Mat3x4 &v, const LMatrix3x4Property &prop) { return **prop == v; }
static bool operator==(const Mat4x3 &v, const LMatrix4x3Property &prop) { return **prop == v; }
static bool operator==(const Mat4 &v, const LMatrix4Property &prop) { return **prop == v; }

template<class TLinkProperty, class TUnderlyingLinkProperty, typename TLink>
void link_bool(lua_State *l, LBoolProperty &prop, TLinkProperty &propOther)
{
	prop->Link<TUnderlyingLinkProperty, TLink>(*propOther, [](const TLink &v) -> bool { return v != 0 ? true : false; });
}

template<class TProperty, typename T, class TLuaClass>
void add_generic_vector_methods(TLuaClass &classDef)
{
	Lua::Property::add_generic_methods<TProperty, T, TLuaClass>(classDef);
	classDef.def(luabind::constructor<>());
	classDef.def(luabind::constructor<T>());

	classDef.def(luabind::tostring(luabind::const_self));
	classDef.def(luabind::self + T());
	classDef.def(luabind::self + luabind::self);
	classDef.def(T() + luabind::self);

	classDef.def(luabind::self - T());
	classDef.def(luabind::self - luabind::self);
	classDef.def(T() - luabind::self);

	classDef.def(luabind::self * float());
	classDef.def(float() * luabind::self);

	classDef.def(luabind::self / float());

	classDef.def(luabind::self == luabind::self);
	classDef.def(luabind::self == T());
	classDef.def(T() == luabind::self);
}

template<class TProperty, typename T, class TLuaClass>
void add_arithmetic_operators(TLuaClass &classDef)
{
	classDef.def(luabind::constructor<>());
	classDef.def(luabind::constructor<T>());
	classDef.def(luabind::tostring(luabind::const_self));
	classDef.def(luabind::self + T());
	classDef.def(luabind::self + luabind::self);
	classDef.def(T() + luabind::self);

	classDef.def(luabind::self - T());
	classDef.def(luabind::self - luabind::self);
	classDef.def(T() - luabind::self);

	classDef.def(luabind::self * T());
	classDef.def(luabind::self * luabind::self);
	classDef.def(T() * luabind::self);

	classDef.def(luabind::self / T());
	classDef.def(luabind::self / luabind::self);
	classDef.def(T() / luabind::self);

	classDef.def(luabind::self == luabind::self);
	classDef.def(luabind::self == T());
	classDef.def(T() == luabind::self);

	classDef.def(luabind::self < T());
	classDef.def(luabind::self < luabind::self);
	classDef.def(T() < luabind::self);
	classDef.def(luabind::self <= T());
	classDef.def(luabind::self <= luabind::self);
	classDef.def(T() <= luabind::self);
}

template<class TProperty, typename T>
void register_property_class(luabind::module_ &mod, const char *name) // Note: MUST be const char*, not std::string!
{
	auto propClassDef = luabind::class_<TProperty, LBasePropertyWrapper>(name);
	add_arithmetic_operators(propClassDef);
	Lua::Property::add_generic_methods<TProperty, T, luabind::class_<TProperty>>(propClassDef);
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LInt8Property &)>(Lua::Property::link_different<TProperty, T, LInt8Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LUInt8Property &)>(Lua::Property::link_different<TProperty, T, LUInt8Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LInt16Property &)>(Lua::Property::link_different<TProperty, T, LInt16Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LUInt16Property &)>(Lua::Property::link_different<TProperty, T, LUInt16Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LInt32Property &)>(Lua::Property::link_different<TProperty, T, LInt32Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LUInt32Property &)>(Lua::Property::link_different<TProperty, T, LUInt32Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LInt64Property &)>(Lua::Property::link_different<TProperty, T, LInt64Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LUInt64Property &)>(Lua::Property::link_different<TProperty, T, LUInt64Property>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LFloatProperty &)>(Lua::Property::link_different<TProperty, T, LFloatProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LDoubleProperty &)>(Lua::Property::link_different<TProperty, T, LDoubleProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LLongDoubleProperty &)>(Lua::Property::link_different<TProperty, T, LLongDoubleProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua_State *, TProperty &, LBoolProperty &)>(Lua::Property::link_different<TProperty, T, LBoolProperty>));
	mod[propClassDef];
}

void Lua::Property::register_classes(Lua::Interface &l)
{
	auto &modUtil = l.RegisterLibrary("util");

	auto basePropDef = luabind::class_<LBasePropertyWrapper>("BaseProperty");
	modUtil[basePropDef];

	/*register_property_class<LInt8Property,int8_t>(modUtil,"UInt32Property");
	register_property_class<LUInt8Property,uint8_t>(modUtil,"UInt8Property");
	register_property_class<LInt16Property,int16_t>(modUtil,"Int16Property");
	register_property_class<LUInt16Property,uint16_t>(modUtil,"UInt16Property");
	register_property_class<LInt32Property,int32_t>(modUtil,"Int32Property");
	register_property_class<LUInt32Property,uint32_t>(modUtil,"UInt32Property");
	register_property_class<LInt64Property,int64_t>(modUtil,"Int64Property");
	register_property_class<LUInt64Property,uint64_t>(modUtil,"UInt64Property");

	register_property_class<LDoubleProperty,double>(modUtil,"DoubleProperty");
	register_property_class<LLongDoubleProperty,long double>(modUtil,"LongDoubleProperty");*/

	auto floatPropDef = luabind::class_<LGenericFloatPropertyWrapper>("FloatProperty");
	add_arithmetic_operators<LGenericFloatPropertyWrapper, double, luabind::class_<LGenericFloatPropertyWrapper>>(floatPropDef);
	add_generic_methods<LGenericFloatPropertyWrapper, double, luabind::class_<LGenericFloatPropertyWrapper>>(floatPropDef);
	floatPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericFloatPropertyWrapper &, LBoolProperty &)>([](lua_State *l, LGenericFloatPropertyWrapper &prop, LBoolProperty &propOther) { prop->Link(*propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericFloatPropertyWrapper &, LStringProperty &)>([](lua_State *l, LGenericFloatPropertyWrapper &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericFloatPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua_State *l, LGenericFloatPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { prop->Link(propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericFloatPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua_State *l, LGenericFloatPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { prop->Link(propOther); }));
	modUtil[floatPropDef];

	auto intPropDef = luabind::class_<LGenericIntPropertyWrapper>("IntProperty");
	add_arithmetic_operators<LGenericIntPropertyWrapper, int32_t, luabind::class_<LGenericIntPropertyWrapper>>(intPropDef);
	add_generic_methods<LGenericIntPropertyWrapper, int32_t, luabind::class_<LGenericIntPropertyWrapper>>(intPropDef);
	intPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericIntPropertyWrapper &, LBoolProperty &)>([](lua_State *l, LGenericIntPropertyWrapper &prop, LBoolProperty &propOther) { prop->Link(*propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericIntPropertyWrapper &, LStringProperty &)>([](lua_State *l, LGenericIntPropertyWrapper &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericIntPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua_State *l, LGenericIntPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { prop->Link(propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua_State *, LGenericIntPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua_State *l, LGenericIntPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { prop->Link(propOther); }));
	modUtil[intPropDef];

	// Boolean
	auto boolDef = luabind::class_<LBoolPropertyWrapper, LBasePropertyWrapper>("BoolProperty");
	boolDef.def(luabind::constructor<>());
	boolDef.def(luabind::constructor<bool>());
	boolDef.def(luabind::tostring(luabind::const_self));

	add_generic_methods<LBoolPropertyWrapper, bool, luabind::class_<LBoolPropertyWrapper, LBasePropertyWrapper>>(boolDef);
	boolDef.def("Link", static_cast<void (*)(lua_State *, LBoolPropertyWrapper &, LBoolPropertyWrapper &)>([](lua_State *l, LBoolPropertyWrapper &prop, LBoolPropertyWrapper &propOther) { prop->Link(*propOther); }));
	boolDef.def("Link",
	  static_cast<void (*)(lua_State *, LBoolPropertyWrapper &, LStringProperty &)>([](lua_State *l, LBoolPropertyWrapper &prop, LStringProperty &propOther) { prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> bool { return util::to_boolean(v); }); }));
	boolDef.def("Link", static_cast<void (*)(lua_State *, LBoolPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua_State *l, LBoolPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	boolDef.def("Link", static_cast<void (*)(lua_State *, LBoolPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua_State *l, LBoolPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	modUtil[boolDef];

	// Color
	auto colDef = luabind::class_<LColorProperty, LBasePropertyWrapper>("ColorProperty");
	colDef.def(luabind::constructor<>());
	colDef.def(luabind::constructor<Color>());
	colDef.def(luabind::constructor<int16_t, int16_t, int16_t, int16_t>());
	colDef.def(luabind::constructor<std::string>());
	colDef.def(luabind::constructor<Vector3>());
	colDef.def(luabind::constructor<Vector4>());

	colDef.def(luabind::tostring(luabind::const_self));
	colDef.def(luabind::self + Color());
	colDef.def(luabind::self + luabind::self);
	colDef.def(Color() + luabind::self);

	colDef.def(luabind::self - Color());
	colDef.def(luabind::self - luabind::self);
	colDef.def(Color() - luabind::self);

	colDef.def(luabind::self * float());
	colDef.def(float() * luabind::self);

	colDef.def(luabind::self / float());

	colDef.def(luabind::self == luabind::self);
	colDef.def(luabind::self == Color());
	colDef.def(Color() == luabind::self);

	add_generic_methods<LColorProperty, Color, luabind::class_<LColorProperty, LBasePropertyWrapper>>(colDef);
	colDef.def("Link", static_cast<void (*)(lua_State *, LColorProperty &, LColorProperty &)>(link<LColorProperty, Color>));
	colDef.def("Link", static_cast<void (*)(lua_State *, LColorProperty &, LStringProperty &)>([](lua_State *l, LColorProperty &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	colDef.def("Link", static_cast<void (*)(lua_State *, LColorProperty &, LVector3Property &)>([](lua_State *l, LColorProperty &prop, LVector3Property &propOther) { prop->Link<util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> Color { return Color {v}; }); }));
	colDef.def("Link", static_cast<void (*)(lua_State *, LColorProperty &, LVector4Property &)>([](lua_State *l, LColorProperty &prop, LVector4Property &propOther) { prop->Link<util::Vector4Property, Vector4>(*propOther, [](const Vector4 &v) -> Color { return Color {v}; }); }));
	/*colDef.def("Link",static_cast<void(*)(lua_State*,LColorProperty&,LStringProperty&)>([](lua_State *l,LColorProperty &prop,LStringProperty &propOther) {
		prop->Link<util::StringProperty,std::string>(*propOther,[](const std::string &v) -> Color {
			return Color{v};
		});
	}));*/
	modUtil[colDef];

	// EulerAngles
	auto angDef = luabind::class_<LEulerAnglesProperty, LBasePropertyWrapper>("EulerAnglesProperty");
	angDef.def(luabind::constructor<>());
	angDef.def(luabind::constructor<EulerAngles>());
	angDef.def(luabind::constructor<float, float, float>());
	angDef.def(luabind::constructor<std::string>());

	angDef.def(luabind::tostring(luabind::const_self));
	angDef.def(luabind::self + EulerAngles());
	angDef.def(luabind::self + luabind::self);
	angDef.def(EulerAngles() + luabind::self);

	angDef.def(luabind::self - EulerAngles());
	angDef.def(luabind::self - luabind::self);
	angDef.def(EulerAngles() - luabind::self);

	angDef.def(luabind::self * float());
	angDef.def(float() * luabind::self);

	angDef.def(luabind::self / float());

	angDef.def(luabind::self == luabind::self);
	angDef.def(luabind::self == EulerAngles());
	angDef.def(EulerAngles() == luabind::self);

	add_generic_methods<LEulerAnglesProperty, EulerAngles, luabind::class_<LEulerAnglesProperty, LBasePropertyWrapper>>(angDef);
	angDef.def("Link", static_cast<void (*)(lua_State *, LEulerAnglesProperty &, LEulerAnglesProperty &)>(link<LEulerAnglesProperty, EulerAngles>));
	angDef.def("Link", static_cast<void (*)(lua_State *, LEulerAnglesProperty &, LStringProperty &)>([](lua_State *l, LEulerAnglesProperty &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	/*angDef.def("Link",static_cast<void(*)(lua_State*,LEulerAnglesProperty&,LStringProperty&)>([](lua_State *l,LEulerAnglesProperty &prop,LStringProperty &propOther) {
		prop->Link<util::StringProperty,std::string>(*propOther,[](const std::string &v) -> EulerAngles {
			return EulerAngles{v};
		});
	}));*/
	modUtil[angDef];

	// Vector
	auto vec3Def = luabind::class_<LVector3Property, LBasePropertyWrapper>("VectorProperty");
	add_generic_vector_methods<LVector3Property, Vector3, luabind::class_<LVector3Property, LBasePropertyWrapper>>(vec3Def);
	vec3Def.def(luabind::constructor<float, float, float>());
	vec3Def.def("Link", static_cast<void (*)(lua_State *, LVector3Property &, LVector3Property &)>(link<LVector3Property, Vector3>));
	vec3Def.def("Link", static_cast<void (*)(lua_State *, LVector3Property &, LColorProperty &)>([](lua_State *l, LVector3Property &prop, LColorProperty &propOther) { prop->Link<util::ColorProperty, Color>(*propOther, [](const Color &v) -> Vector3 { return v.ToVector3(); }); }));
	vec3Def.def("Link",
	  static_cast<void (*)(lua_State *, LVector3Property &, LVector3iProperty &)>([](lua_State *l, LVector3Property &prop, LVector3iProperty &propOther) { prop->Link<util::Vector3iProperty, Vector3i>(*propOther, [](const Vector3i &v) -> Vector3 { return Vector3(v.x, v.y, v.z); }); }));
	vec3Def.def("Link",
	  static_cast<void (*)(lua_State *, LVector3Property &, LStringProperty &)>([](lua_State *l, LVector3Property &prop, LStringProperty &propOther) { prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector3 { return uvec::create(v); }); }));
	modUtil[vec3Def];

	auto vec3iDef = luabind::class_<LVector3iProperty, LBasePropertyWrapper>("VectoriProperty");
	add_generic_vector_methods<LVector3iProperty, Vector3i, luabind::class_<LVector3iProperty, LBasePropertyWrapper>>(vec3iDef);
	vec3iDef.def(luabind::constructor<int32_t, int32_t, int32_t>());
	vec3iDef.def("Link", static_cast<void (*)(lua_State *, LVector3iProperty &, LVector3iProperty &)>(link<LVector3iProperty, Vector3i>));
	vec3iDef.def("Link",
	  static_cast<void (*)(lua_State *, LVector3iProperty &, LVector3Property &)>([](lua_State *l, LVector3iProperty &prop, LVector3Property &propOther) { prop->Link<util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> Vector3i { return Vector3i(v.x, v.y, v.z); }); }));
	vec3iDef.def("Link", static_cast<void (*)(lua_State *, LVector3iProperty &, LStringProperty &)>([](lua_State *l, LVector3iProperty &prop, LStringProperty &propOther) {
		prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector3i {
			Vector3i r;
			ustring::string_to_array<decltype(r)::value_type, int32_t>(v, &r[0], atoi, 3);
			return r;
		});
	}));
	modUtil[vec3iDef];

	auto vec2Def = luabind::class_<LVector2Property, LBasePropertyWrapper>("Vector2Property");
	add_generic_vector_methods<LVector2Property, Vector2, luabind::class_<LVector2Property, LBasePropertyWrapper>>(vec2Def);
	vec2Def.def(luabind::constructor<float, float>());
	vec2Def.def("Link", static_cast<void (*)(lua_State *, LVector2Property &, LVector2Property &)>(link<LVector2Property, Vector2>));
	vec2Def.def("Link",
	  static_cast<void (*)(lua_State *, LVector2Property &, LVector2iProperty &)>([](lua_State *l, LVector2Property &prop, LVector2iProperty &propOther) { prop->Link<util::Vector2iProperty, Vector2i>(*propOther, [](const Vector2i &v) -> Vector2 { return Vector2(v.x, v.y); }); }));
	vec2Def.def("Link", static_cast<void (*)(lua_State *, LVector2Property &, LStringProperty &)>([](lua_State *l, LVector2Property &prop, LStringProperty &propOther) {
		prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector2 {
			Vector2 r;
			ustring::string_to_array<decltype(r)::value_type, Double>(v, &r[0], atof, 2);
			return r;
		});
	}));
	modUtil[vec2Def];

	auto vec2iDef = luabind::class_<LVector2iProperty, LBasePropertyWrapper>("Vector2iProperty");
	add_generic_vector_methods<LVector2iProperty, Vector2i, luabind::class_<LVector2iProperty, LBasePropertyWrapper>>(vec2iDef);
	vec2iDef.def(luabind::constructor<int32_t, int32_t>());
	vec2iDef.def("Link", static_cast<void (*)(lua_State *, LVector2iProperty &, LVector2iProperty &)>(link<LVector2iProperty, Vector2i>));
	vec2iDef.def("Link",
	  static_cast<void (*)(lua_State *, LVector2iProperty &, LVector2Property &)>([](lua_State *l, LVector2iProperty &prop, LVector2Property &propOther) { prop->Link<util::Vector2Property, Vector2>(*propOther, [](const Vector2 &v) -> Vector2i { return Vector2i(v.x, v.y); }); }));
	vec2iDef.def("Link", static_cast<void (*)(lua_State *, LVector2iProperty &, LStringProperty &)>([](lua_State *l, LVector2iProperty &prop, LStringProperty &propOther) {
		prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector2i {
			Vector2i r;
			ustring::string_to_array<decltype(r)::value_type, int32_t>(v, &r[0], atoi, 2);
			return r;
		});
	}));
	modUtil[vec2iDef];

	auto vec4Def = luabind::class_<LVector4Property, LBasePropertyWrapper>("Vector4Property");
	add_generic_vector_methods<LVector4Property, Vector4, luabind::class_<LVector4Property, LBasePropertyWrapper>>(vec4Def);
	vec4Def.def(luabind::constructor<float, float, float, float>());
	vec4Def.def("Link", static_cast<void (*)(lua_State *, LVector4Property &, LVector4Property &)>(link<LVector4Property, Vector4>));
	vec4Def.def("Link", static_cast<void (*)(lua_State *, LVector4Property &, LVector4iProperty &)>([](lua_State *l, LVector4Property &prop, LVector4iProperty &propOther) {
		prop->Link<util::Vector4iProperty, Vector4i>(*propOther, [](const Vector4i &v) -> Vector4 { return Vector4(v.x, v.y, v.z, v.w); });
	}));
	vec4Def.def("Link", static_cast<void (*)(lua_State *, LVector4Property &, LStringProperty &)>([](lua_State *l, LVector4Property &prop, LStringProperty &propOther) {
		prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector4 {
			Vector4 r;
			ustring::string_to_array<decltype(r)::value_type, Double>(v, &r[0], atof, 4);
			return r;
		});
	}));
	modUtil[vec4Def];

	auto vec4iDef = luabind::class_<LVector4iProperty, LBasePropertyWrapper>("Vector4iProperty");
	add_generic_vector_methods<LVector4iProperty, Vector4i, luabind::class_<LVector4iProperty, LBasePropertyWrapper>>(vec4iDef);
	vec4iDef.def(luabind::constructor<int32_t, int32_t, int32_t, int32_t>());
	vec4iDef.def("Link", static_cast<void (*)(lua_State *, LVector4iProperty &, LVector4iProperty &)>(link<LVector4iProperty, Vector4i>));
	vec4iDef.def("Link",
	  static_cast<void (*)(lua_State *, LVector4iProperty &, LVector4Property &)>([](lua_State *l, LVector4iProperty &prop, LVector4Property &propOther) { prop->Link<util::Vector4Property, Vector4>(*propOther, [](const Vector4 &v) -> Vector4i { return Vector4i(v.x, v.y, v.z, v.w); }); }));
	vec4iDef.def("Link", static_cast<void (*)(lua_State *, LVector4iProperty &, LStringProperty &)>([](lua_State *l, LVector4iProperty &prop, LStringProperty &propOther) {
		prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector4i {
			Vector4i r;
			ustring::string_to_array<decltype(r)::value_type, int32_t>(v, &r[0], atoi, 4);
			return r;
		});
	}));
	modUtil[vec4iDef];

	// Quaternion
	auto quatDef = luabind::class_<LQuatProperty, LBasePropertyWrapper>("QuaternionProperty");
	add_generic_methods<LQuatProperty, Quat, luabind::class_<LQuatProperty, LBasePropertyWrapper>>(quatDef);
	quatDef.def(luabind::constructor<>());
	quatDef.def(luabind::constructor<float, float, float, float>());
	quatDef.def(luabind::tostring(luabind::const_self));
	quatDef.def("Link", static_cast<void (*)(lua_State *, LQuatProperty &, LQuatProperty &)>(link<LQuatProperty, Quat>));
	quatDef.def("Link", static_cast<void (*)(lua_State *, LQuatProperty &, LStringProperty &)>([](lua_State *l, LQuatProperty &prop, LStringProperty &propOther) { prop->Link<util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Quat { return uquat::create(v); }); }));
	modUtil[quatDef];

	// String
	auto strDef = luabind::class_<LStringProperty, LBasePropertyWrapper>("StringProperty");
	add_generic_methods<LStringProperty, std::string, luabind::class_<LStringProperty, LBasePropertyWrapper>>(strDef);
	strDef.def(luabind::constructor<>());
	strDef.def(luabind::constructor<std::string>());
	strDef.def(luabind::tostring(luabind::const_self));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LStringProperty &)>(link<LStringProperty, std::string>));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LBoolProperty &)>([](lua_State *l, LStringProperty &prop, LBoolProperty &propOther) { prop->Link<util::BoolProperty, bool>(*propOther, [](const bool &v) -> std::string { return v ? "true" : "false"; }); }));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LColorPropertyWrapper &)>([](lua_State *l, LStringProperty &prop, LColorPropertyWrapper &propOther) {
		prop->Link<util::ColorProperty, Color>(*propOther, [](const Color &v) -> std::string { return std::to_string(v.r) + " " + std::to_string(v.g) + " " + std::to_string(v.b) + " " + std::to_string(v.a); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LGenericFloatPropertyWrapper &)>([](lua_State *l, LStringProperty &prop, LGenericFloatPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LGenericIntPropertyWrapper &)>([](lua_State *l, LStringProperty &prop, LGenericIntPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector2Property &)>([](lua_State *l, LStringProperty &prop, LVector2Property &propOther) {
		prop->Link<util::Vector2Property, Vector2>(*propOther, [](const Vector2 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector2iProperty &)>([](lua_State *l, LStringProperty &prop, LVector2iProperty &propOther) {
		prop->Link<util::Vector2iProperty, Vector2i>(*propOther, [](const Vector2i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector3Property &)>([](lua_State *l, LStringProperty &prop, LVector3Property &propOther) {
		prop->Link<util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector3iProperty &)>([](lua_State *l, LStringProperty &prop, LVector3iProperty &propOther) {
		prop->Link<util::Vector3iProperty, Vector3i>(*propOther, [](const Vector3i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector4Property &)>([](lua_State *l, LStringProperty &prop, LVector4Property &propOther) {
		prop->Link<util::Vector4Property, Vector4>(*propOther, [](const Vector4 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LVector4iProperty &)>([](lua_State *l, LStringProperty &prop, LVector4iProperty &propOther) {
		prop->Link<util::Vector4iProperty, Vector4i>(*propOther, [](const Vector4i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LQuatProperty &)>([](lua_State *l, LStringProperty &prop, LQuatProperty &propOther) {
		prop->Link<util::QuatProperty, Quat>(*propOther, [](const Quat &v) -> std::string { return std::to_string(v.w) + " " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua_State *, LStringProperty &, LEulerAnglesProperty &)>([](lua_State *l, LStringProperty &prop, LEulerAnglesProperty &propOther) {
		prop->Link<util::EulerAnglesProperty, EulerAngles>(*propOther, [](const EulerAngles &v) -> std::string { return std::to_string(v.p) + " " + std::to_string(v.y) + " " + std::to_string(v.r); });
	}));
	modUtil[strDef];

	// Matrix
	auto mat2Def = luabind::class_<LMatrix2Property, LBasePropertyWrapper>("Mat2Property");
	add_generic_methods<LMatrix2Property, Mat2, luabind::class_<LMatrix2Property, LBasePropertyWrapper>>(mat2Def);
	mat2Def.def(luabind::constructor<>());
	mat2Def.def(luabind::constructor<Mat2>());
	mat2Def.def(luabind::tostring(luabind::const_self));
	mat2Def.def("Link", static_cast<void (*)(lua_State *, LMatrix2Property &, LMatrix2Property &)>(link<LMatrix2Property, Mat2>));
	modUtil[mat2Def];

	auto mat2x3Def = luabind::class_<LMatrix2x3Property, LBasePropertyWrapper>("Mat2x3Property");
	add_generic_methods<LMatrix2x3Property, Mat2x3, luabind::class_<LMatrix2x3Property, LBasePropertyWrapper>>(mat2x3Def);
	mat2x3Def.def(luabind::constructor<>());
	mat2x3Def.def(luabind::constructor<Mat2x3>());
	mat2x3Def.def(luabind::tostring(luabind::const_self));
	mat2x3Def.def("Link", static_cast<void (*)(lua_State *, LMatrix2x3Property &, LMatrix2x3Property &)>(link<LMatrix2x3Property, Mat2x3>));
	modUtil[mat2x3Def];

	auto mat3x2Def = luabind::class_<LMatrix3x2Property, LBasePropertyWrapper>("Mat3x2Property");
	add_generic_methods<LMatrix3x2Property, Mat3x2, luabind::class_<LMatrix3x2Property, LBasePropertyWrapper>>(mat3x2Def);
	mat3x2Def.def(luabind::constructor<>());
	mat3x2Def.def(luabind::constructor<Mat3x2>());
	mat3x2Def.def(luabind::tostring(luabind::const_self));
	mat3x2Def.def("Link", static_cast<void (*)(lua_State *, LMatrix3x2Property &, LMatrix3x2Property &)>(link<LMatrix3x2Property, Mat3x2>));
	modUtil[mat3x2Def];

	auto mat3Def = luabind::class_<LMatrix3Property, LBasePropertyWrapper>("Mat3Property");
	add_generic_methods<LMatrix3Property, Mat3, luabind::class_<LMatrix3Property, LBasePropertyWrapper>>(mat3Def);
	mat3Def.def(luabind::constructor<>());
	mat3Def.def(luabind::constructor<Mat3>());
	mat3Def.def(luabind::tostring(luabind::const_self));
	mat3Def.def("Link", static_cast<void (*)(lua_State *, LMatrix3Property &, LMatrix3Property &)>(link<LMatrix3Property, Mat3>));
	modUtil[mat3Def];

	auto mat3x4Def = luabind::class_<LMatrix3x4Property, LBasePropertyWrapper>("Mat3x4Property");
	add_generic_methods<LMatrix3x4Property, Mat3x4, luabind::class_<LMatrix3x4Property, LBasePropertyWrapper>>(mat3x4Def);
	mat3x4Def.def(luabind::constructor<>());
	mat3x4Def.def(luabind::constructor<Mat3x4>());
	mat3x4Def.def(luabind::tostring(luabind::const_self));
	mat3x4Def.def("Link", static_cast<void (*)(lua_State *, LMatrix3x4Property &, LMatrix3x4Property &)>(link<LMatrix3x4Property, Mat3x4>));
	modUtil[mat3x4Def];

	auto mat4x3Def = luabind::class_<LMatrix4x3Property, LBasePropertyWrapper>("Mat4x3Property");
	add_generic_methods<LMatrix4x3Property, Mat4x3, luabind::class_<LMatrix4x3Property, LBasePropertyWrapper>>(mat4x3Def);
	mat4x3Def.def(luabind::constructor<>());
	mat4x3Def.def(luabind::constructor<Mat4x3>());
	mat4x3Def.def(luabind::tostring(luabind::const_self));
	mat4x3Def.def("Link", static_cast<void (*)(lua_State *, LMatrix4x3Property &, LMatrix4x3Property &)>(link<LMatrix4x3Property, Mat4x3>));
	modUtil[mat4x3Def];

	auto mat4Def = luabind::class_<LMatrix4Property, LBasePropertyWrapper>("Mat4Property");
	add_generic_methods<LMatrix4Property, Mat4, luabind::class_<LMatrix4Property, LBasePropertyWrapper>>(mat4Def);
	mat4Def.def(luabind::constructor<>());
	mat4Def.def(luabind::constructor<Mat4>());
	mat4Def.def(luabind::tostring(luabind::const_self));
	mat4Def.def("Link", static_cast<void (*)(lua_State *, LMatrix4Property &, LMatrix4Property &)>(link<LMatrix4Property, Mat4>));
	modUtil[mat4Def];
}

void Lua::Property::push(lua_State *l, util::Int8Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::UInt8Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Int16Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::UInt16Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Int32Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::UInt32Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Int64Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::UInt64Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::FloatProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::DoubleProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::LongDoubleProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::BoolProperty &prop) { push_property<LBoolPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::ColorProperty &prop) { push_property<LColorPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, ::util::EulerAnglesProperty &prop) { push_property<LEulerAnglesPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector2Property &prop) { push_property<LVector2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector2iProperty &prop) { push_property<LVector2iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector3Property &prop) { push_property<LVector3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector3iProperty &prop) { push_property<LVector3iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector4Property &prop) { push_property<LVector4PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Vector4iProperty &prop) { push_property<LVector4iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::QuatProperty &prop) { push_property<LQuatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::StringProperty &prop) { push_property<LStringPropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix2Property &prop) { push_property<LMatrix2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix2x3Property &prop) { push_property<LMatrix2x3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix3x2Property &prop) { push_property<LMatrix3x2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix3Property &prop) { push_property<LMatrix3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix3x4Property &prop) { push_property<LMatrix3x4PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix4x3Property &prop) { push_property<LMatrix4x3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua_State *l, util::Matrix4Property &prop) { push_property<LMatrix4PropertyWrapper>(l, prop); }

void Lua::Property::push(lua_State *l, ::util::BaseProperty &prop)
{
	auto typeIndex = std::type_index(typeid(prop));
	if(typeIndex == std::type_index(typeid(::util::Int8Property)))
		push(l, static_cast<::util::Int8Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::UInt8Property)))
		push(l, static_cast<::util::UInt8Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Int16Property)))
		push(l, static_cast<::util::Int16Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::UInt16Property)))
		push(l, static_cast<::util::UInt16Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Int32Property)))
		push(l, static_cast<::util::Int32Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::UInt32Property)))
		push(l, static_cast<::util::UInt32Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Int64Property)))
		push(l, static_cast<::util::Int64Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::UInt64Property)))
		push(l, static_cast<::util::UInt64Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::FloatProperty)))
		push(l, static_cast<::util::FloatProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::DoubleProperty)))
		push(l, static_cast<::util::DoubleProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::LongDoubleProperty)))
		push(l, static_cast<::util::LongDoubleProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::BoolProperty)))
		push(l, static_cast<::util::BoolProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::ColorProperty)))
		push(l, static_cast<::util::ColorProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::EulerAnglesProperty)))
		push(l, static_cast<::util::EulerAnglesProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector2Property)))
		push(l, static_cast<::util::Vector2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector2iProperty)))
		push(l, static_cast<::util::Vector2iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector3Property)))
		push(l, static_cast<::util::Vector3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector3iProperty)))
		push(l, static_cast<::util::Vector3iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector4Property)))
		push(l, static_cast<::util::Vector4Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Vector4iProperty)))
		push(l, static_cast<::util::Vector4iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::QuatProperty)))
		push(l, static_cast<::util::QuatProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::StringProperty)))
		push(l, static_cast<::util::StringProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::EntityProperty)))
		push(l, static_cast<pragma::EntityProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix2Property)))
		push(l, static_cast<::util::Matrix2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix2x3Property)))
		push(l, static_cast<::util::Matrix2x3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix3x2Property)))
		push(l, static_cast<::util::Matrix3x2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix3Property)))
		push(l, static_cast<::util::Matrix3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix3x4Property)))
		push(l, static_cast<::util::Matrix3x4Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix4x3Property)))
		push(l, static_cast<::util::Matrix4x3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(::util::Matrix4Property)))
		push(l, static_cast<::util::Matrix4Property &>(prop));
	else
		throw std::logic_error("Unregistered property type!");
}
