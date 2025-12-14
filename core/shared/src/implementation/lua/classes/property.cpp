// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.property;

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

static pragma::math::Transform operator*(pragma::math::Transform v, const LTransformPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(pragma::math::Transform v, const LTransformPropertyWrapper &prop) { return **prop == v; }
static std::ostream &operator<<(std::ostream &str, const LTransformPropertyWrapper &v) { return str << **v; }

//////////

static pragma::math::ScaledTransform operator*(pragma::math::ScaledTransform v, const LScaledTransformPropertyWrapper &prop) { return v * prop->GetValue(); }
static bool operator==(pragma::math::ScaledTransform v, const LScaledTransformPropertyWrapper &prop) { return **prop == v; }
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
void link_bool(lua::State *l, LBoolProperty &prop, TLinkProperty &propOther)
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
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LInt8Property &)>(Lua::Property::link_different<TProperty, T, LInt8Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LUInt8Property &)>(Lua::Property::link_different<TProperty, T, LUInt8Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LInt16Property &)>(Lua::Property::link_different<TProperty, T, LInt16Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LUInt16Property &)>(Lua::Property::link_different<TProperty, T, LUInt16Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LInt32Property &)>(Lua::Property::link_different<TProperty, T, LInt32Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LUInt32Property &)>(Lua::Property::link_different<TProperty, T, LUInt32Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LInt64Property &)>(Lua::Property::link_different<TProperty, T, LInt64Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LUInt64Property &)>(Lua::Property::link_different<TProperty, T, LUInt64Property>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LFloatProperty &)>(Lua::Property::link_different<TProperty, T, LFloatProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LDoubleProperty &)>(Lua::Property::link_different<TProperty, T, LDoubleProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LLongDoubleProperty &)>(Lua::Property::link_different<TProperty, T, LLongDoubleProperty>));
	propClassDef.def("Link", static_cast<void (*)(lua::State *, TProperty &, LBoolProperty &)>(Lua::Property::link_different<TProperty, T, LBoolProperty>));
	mod[propClassDef];
}

void Lua::Property::register_classes(Interface &l)
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
	floatPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericFloatPropertyWrapper &, LBoolProperty &)>([](lua::State *l, LGenericFloatPropertyWrapper &prop, LBoolProperty &propOther) { prop->Link(*propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericFloatPropertyWrapper &, LStringProperty &)>([](lua::State *l, LGenericFloatPropertyWrapper &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericFloatPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua::State *l, LGenericFloatPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { prop->Link(propOther); }));
	floatPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericFloatPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua::State *l, LGenericFloatPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { prop->Link(propOther); }));
	modUtil[floatPropDef];

	auto intPropDef = luabind::class_<LGenericIntPropertyWrapper>("IntProperty");
	add_arithmetic_operators<LGenericIntPropertyWrapper, int32_t, luabind::class_<LGenericIntPropertyWrapper>>(intPropDef);
	add_generic_methods<LGenericIntPropertyWrapper, int32_t, luabind::class_<LGenericIntPropertyWrapper>>(intPropDef);
	intPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericIntPropertyWrapper &, LBoolProperty &)>([](lua::State *l, LGenericIntPropertyWrapper &prop, LBoolProperty &propOther) { prop->Link(*propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericIntPropertyWrapper &, LStringProperty &)>([](lua::State *l, LGenericIntPropertyWrapper &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericIntPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua::State *l, LGenericIntPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { prop->Link(propOther); }));
	intPropDef.def("Link", static_cast<void (*)(lua::State *, LGenericIntPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua::State *l, LGenericIntPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { prop->Link(propOther); }));
	modUtil[intPropDef];

	// Boolean
	auto boolDef = luabind::class_<LBoolPropertyWrapper, LBasePropertyWrapper>("BoolProperty");
	boolDef.def(luabind::constructor<>());
	boolDef.def(luabind::constructor<bool>());
	boolDef.def(luabind::tostring(luabind::const_self));

	add_generic_methods<LBoolPropertyWrapper, bool, luabind::class_<LBoolPropertyWrapper, LBasePropertyWrapper>>(boolDef);
	boolDef.def("Link", static_cast<void (*)(lua::State *, LBoolPropertyWrapper &, LBoolPropertyWrapper &)>([](lua::State *l, LBoolPropertyWrapper &prop, LBoolPropertyWrapper &propOther) { prop->Link(*propOther); }));
	boolDef.def("Link", static_cast<void (*)(lua::State *, LBoolPropertyWrapper &, LStringProperty &)>([](lua::State *l, LBoolPropertyWrapper &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> bool { return pragma::util::to_boolean(v); });
	}));
	boolDef.def("Link", static_cast<void (*)(lua::State *, LBoolPropertyWrapper &, LGenericFloatPropertyWrapper &)>([](lua::State *l, LBoolPropertyWrapper &prop, LGenericFloatPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	boolDef.def("Link", static_cast<void (*)(lua::State *, LBoolPropertyWrapper &, LGenericIntPropertyWrapper &)>([](lua::State *l, LBoolPropertyWrapper &prop, LGenericIntPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	modUtil[boolDef];

	// Color
	auto colDef = luabind::class_<LColorProperty, LBasePropertyWrapper>("ColorProperty");
	colDef.def(luabind::constructor<>());
	colDef.def(luabind::constructor<::Color>());
	colDef.def(luabind::constructor<int16_t, int16_t, int16_t, int16_t>());
	colDef.def(luabind::constructor<std::string>());
	colDef.def(luabind::constructor<Vector3>());
	colDef.def(luabind::constructor<::Vector4>());

	colDef.def(luabind::tostring(luabind::const_self));
	colDef.def(luabind::self + ::Color());
	colDef.def(luabind::self + luabind::self);
	colDef.def(::Color() + luabind::self);

	colDef.def(luabind::self - ::Color());
	colDef.def(luabind::self - luabind::self);
	colDef.def(::Color() - luabind::self);

	colDef.def(luabind::self * float());
	colDef.def(float() * luabind::self);

	colDef.def(luabind::self / float());

	colDef.def(luabind::self == luabind::self);
	colDef.def(luabind::self == ::Color());
	colDef.def(::Color() == luabind::self);

	add_generic_methods<LColorProperty, ::Color, luabind::class_<LColorProperty, LBasePropertyWrapper>>(colDef);
	colDef.def("Link", static_cast<void (*)(lua::State *, LColorProperty &, LColorProperty &)>(link<LColorProperty, ::Color>));
	colDef.def("Link", static_cast<void (*)(lua::State *, LColorProperty &, LStringProperty &)>([](lua::State *l, LColorProperty &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	colDef.def("Link", static_cast<void (*)(lua::State *, LColorProperty &, LVector3Property &)>([](lua::State *l, LColorProperty &prop, LVector3Property &propOther) { prop->Link<pragma::util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> ::Color { return ::Color {v}; }); }));
	colDef.def("Link",
	  static_cast<void (*)(lua::State *, LColorProperty &, LVector4Property &)>([](lua::State *l, LColorProperty &prop, LVector4Property &propOther) { prop->Link<pragma::util::Vector4Property, ::Vector4>(*propOther, [](const ::Vector4 &v) -> ::Color { return ::Color {v}; }); }));
	/*colDef.def("Link",static_cast<void(*)(lua::State*,LColorProperty&,LStringProperty&)>([](lua::State *l,LColorProperty &prop,LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty,std::string>(*propOther,[](const std::string &v) -> Color {
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
	angDef.def("Link", static_cast<void (*)(lua::State *, LEulerAnglesProperty &, LEulerAnglesProperty &)>(link<LEulerAnglesProperty, EulerAngles>));
	angDef.def("Link", static_cast<void (*)(lua::State *, LEulerAnglesProperty &, LStringProperty &)>([](lua::State *l, LEulerAnglesProperty &prop, LStringProperty &propOther) { prop->Link(*propOther); }));
	/*angDef.def("Link",static_cast<void(*)(lua::State*,LEulerAnglesProperty&,LStringProperty&)>([](lua::State *l,LEulerAnglesProperty &prop,LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty,std::string>(*propOther,[](const std::string &v) -> EulerAngles {
			return EulerAngles{v};
		});
	}));*/
	modUtil[angDef];

	// Vector
	auto vec3Def = luabind::class_<LVector3Property, LBasePropertyWrapper>("VectorProperty");
	add_generic_vector_methods<LVector3Property, Vector3, luabind::class_<LVector3Property, LBasePropertyWrapper>>(vec3Def);
	vec3Def.def(luabind::constructor<float, float, float>());
	vec3Def.def("Link", static_cast<void (*)(lua::State *, LVector3Property &, LVector3Property &)>(link<LVector3Property, Vector3>));
	vec3Def.def("Link", static_cast<void (*)(lua::State *, LVector3Property &, LColorProperty &)>([](lua::State *l, LVector3Property &prop, LColorProperty &propOther) { prop->Link<pragma::util::ColorProperty, ::Color>(*propOther, [](const ::Color &v) -> Vector3 { return v.ToVector3(); }); }));
	vec3Def.def("Link",
	  static_cast<void (*)(lua::State *, LVector3Property &, LVector3iProperty &)>([](lua::State *l, LVector3Property &prop, LVector3iProperty &propOther) { prop->Link<pragma::util::Vector3iProperty, Vector3i>(*propOther, [](const Vector3i &v) -> Vector3 { return Vector3(v.x, v.y, v.z); }); }));
	vec3Def.def("Link",
	  static_cast<void (*)(lua::State *, LVector3Property &, LStringProperty &)>([](lua::State *l, LVector3Property &prop, LStringProperty &propOther) { prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector3 { return uvec::create(v); }); }));
	modUtil[vec3Def];

	auto vec3iDef = luabind::class_<LVector3iProperty, LBasePropertyWrapper>("VectoriProperty");
	add_generic_vector_methods<LVector3iProperty, Vector3i, luabind::class_<LVector3iProperty, LBasePropertyWrapper>>(vec3iDef);
	vec3iDef.def(luabind::constructor<int32_t, int32_t, int32_t>());
	vec3iDef.def("Link", static_cast<void (*)(lua::State *, LVector3iProperty &, LVector3iProperty &)>(link<LVector3iProperty, Vector3i>));
	vec3iDef.def("Link",
	  static_cast<void (*)(lua::State *, LVector3iProperty &, LVector3Property &)>([](lua::State *l, LVector3iProperty &prop, LVector3Property &propOther) { prop->Link<pragma::util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> Vector3i { return Vector3i(v.x, v.y, v.z); }); }));
	vec3iDef.def("Link", static_cast<void (*)(lua::State *, LVector3iProperty &, LStringProperty &)>([](lua::State *l, LVector3iProperty &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Vector3i {
			Vector3i r;
			pragma::string::string_to_array<decltype(r)::value_type>(v, &r[0], pragma::string::cstring_to_number<int32_t>, 3);
			return r;
		});
	}));
	modUtil[vec3iDef];

	auto vec2Def = luabind::class_<LVector2Property, LBasePropertyWrapper>("Vector2Property");
	add_generic_vector_methods<LVector2Property, ::Vector2, luabind::class_<LVector2Property, LBasePropertyWrapper>>(vec2Def);
	vec2Def.def(luabind::constructor<float, float>());
	vec2Def.def("Link", static_cast<void (*)(lua::State *, LVector2Property &, LVector2Property &)>(link<LVector2Property, ::Vector2>));
	vec2Def.def("Link", static_cast<void (*)(lua::State *, LVector2Property &, LVector2iProperty &)>([](lua::State *l, LVector2Property &prop, LVector2iProperty &propOther) {
		prop->Link<pragma::util::Vector2iProperty, ::Vector2i>(*propOther, [](const ::Vector2i &v) -> ::Vector2 { return ::Vector2(v.x, v.y); });
	}));
	vec2Def.def("Link", static_cast<void (*)(lua::State *, LVector2Property &, LStringProperty &)>([](lua::State *l, LVector2Property &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> ::Vector2 {
			::Vector2 r;
			pragma::string::string_to_array<decltype(r)::value_type>(v, &r[0], pragma::string::cstring_to_number<float>, 2);
			return r;
		});
	}));
	modUtil[vec2Def];

	auto vec2iDef = luabind::class_<LVector2iProperty, LBasePropertyWrapper>("Vector2iProperty");
	add_generic_vector_methods<LVector2iProperty, ::Vector2i, luabind::class_<LVector2iProperty, LBasePropertyWrapper>>(vec2iDef);
	vec2iDef.def(luabind::constructor<int32_t, int32_t>());
	vec2iDef.def("Link", static_cast<void (*)(lua::State *, LVector2iProperty &, LVector2iProperty &)>(link<LVector2iProperty, ::Vector2i>));
	vec2iDef.def("Link", static_cast<void (*)(lua::State *, LVector2iProperty &, LVector2Property &)>([](lua::State *l, LVector2iProperty &prop, LVector2Property &propOther) {
		prop->Link<pragma::util::Vector2Property, ::Vector2>(*propOther, [](const ::Vector2 &v) -> ::Vector2i { return ::Vector2i(v.x, v.y); });
	}));
	vec2iDef.def("Link", static_cast<void (*)(lua::State *, LVector2iProperty &, LStringProperty &)>([](lua::State *l, LVector2iProperty &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> ::Vector2i {
			::Vector2i r;
			pragma::string::string_to_array<decltype(r)::value_type>(v, &r[0], pragma::string::cstring_to_number<int32_t>, 2);
			return r;
		});
	}));
	modUtil[vec2iDef];

	auto vec4Def = luabind::class_<LVector4Property, LBasePropertyWrapper>("Vector4Property");
	add_generic_vector_methods<LVector4Property, ::Vector4, luabind::class_<LVector4Property, LBasePropertyWrapper>>(vec4Def);
	vec4Def.def(luabind::constructor<float, float, float, float>());
	vec4Def.def("Link", static_cast<void (*)(lua::State *, LVector4Property &, LVector4Property &)>(link<LVector4Property, ::Vector4>));
	vec4Def.def("Link", static_cast<void (*)(lua::State *, LVector4Property &, LVector4iProperty &)>([](lua::State *l, LVector4Property &prop, LVector4iProperty &propOther) {
		prop->Link<pragma::util::Vector4iProperty, ::Vector4i>(*propOther, [](const ::Vector4i &v) -> ::Vector4 { return ::Vector4(v.x, v.y, v.z, v.w); });
	}));
	vec4Def.def("Link", static_cast<void (*)(lua::State *, LVector4Property &, LStringProperty &)>([](lua::State *l, LVector4Property &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> ::Vector4 {
			::Vector4 r;
			pragma::string::string_to_array<decltype(r)::value_type>(v, &r[0], pragma::string::cstring_to_number<float>, 4);
			return r;
		});
	}));
	modUtil[vec4Def];

	auto vec4iDef = luabind::class_<LVector4iProperty, LBasePropertyWrapper>("Vector4iProperty");
	add_generic_vector_methods<LVector4iProperty, ::Vector4i, luabind::class_<LVector4iProperty, LBasePropertyWrapper>>(vec4iDef);
	vec4iDef.def(luabind::constructor<int32_t, int32_t, int32_t, int32_t>());
	vec4iDef.def("Link", static_cast<void (*)(lua::State *, LVector4iProperty &, LVector4iProperty &)>(link<LVector4iProperty, ::Vector4i>));
	vec4iDef.def("Link", static_cast<void (*)(lua::State *, LVector4iProperty &, LVector4Property &)>([](lua::State *l, LVector4iProperty &prop, LVector4Property &propOther) {
		prop->Link<pragma::util::Vector4Property, ::Vector4>(*propOther, [](const ::Vector4 &v) -> ::Vector4i { return ::Vector4i(v.x, v.y, v.z, v.w); });
	}));
	vec4iDef.def("Link", static_cast<void (*)(lua::State *, LVector4iProperty &, LStringProperty &)>([](lua::State *l, LVector4iProperty &prop, LStringProperty &propOther) {
		prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> ::Vector4i {
			::Vector4i r;
			pragma::string::string_to_array<decltype(r)::value_type>(v, &r[0], pragma::string::cstring_to_number<int32_t>, 4);
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
	quatDef.def("Link", static_cast<void (*)(lua::State *, LQuatProperty &, LQuatProperty &)>(link<LQuatProperty, Quat>));
	quatDef.def("Link",
	  static_cast<void (*)(lua::State *, LQuatProperty &, LStringProperty &)>([](lua::State *l, LQuatProperty &prop, LStringProperty &propOther) { prop->Link<pragma::util::StringProperty, std::string>(*propOther, [](const std::string &v) -> Quat { return uquat::create(v); }); }));
	modUtil[quatDef];

	// String
	auto strDef = luabind::class_<LStringProperty, LBasePropertyWrapper>("StringProperty");
	add_generic_methods<LStringProperty, std::string, luabind::class_<LStringProperty, LBasePropertyWrapper>>(strDef);
	strDef.def(luabind::constructor<>());
	strDef.def(luabind::constructor<std::string>());
	strDef.def(luabind::tostring(luabind::const_self));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LStringProperty &)>(link<LStringProperty, std::string>));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LBoolProperty &)>([](lua::State *l, LStringProperty &prop, LBoolProperty &propOther) { prop->Link<pragma::util::BoolProperty, bool>(*propOther, [](const bool &v) -> std::string { return v ? "true" : "false"; }); }));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LColorPropertyWrapper &)>([](lua::State *l, LStringProperty &prop, LColorPropertyWrapper &propOther) {
		prop->Link<pragma::util::ColorProperty, ::Color>(*propOther, [](const ::Color &v) -> std::string { return std::to_string(v.r) + " " + std::to_string(v.g) + " " + std::to_string(v.b) + " " + std::to_string(v.a); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LGenericFloatPropertyWrapper &)>([](lua::State *l, LStringProperty &prop, LGenericFloatPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LGenericIntPropertyWrapper &)>([](lua::State *l, LStringProperty &prop, LGenericIntPropertyWrapper &propOther) { propOther->LinkOther(*prop); }));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector2Property &)>([](lua::State *l, LStringProperty &prop, LVector2Property &propOther) {
		prop->Link<pragma::util::Vector2Property, ::Vector2>(*propOther, [](const ::Vector2 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector2iProperty &)>([](lua::State *l, LStringProperty &prop, LVector2iProperty &propOther) {
		prop->Link<pragma::util::Vector2iProperty, ::Vector2i>(*propOther, [](const ::Vector2i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector3Property &)>([](lua::State *l, LStringProperty &prop, LVector3Property &propOther) {
		prop->Link<pragma::util::Vector3Property, Vector3>(*propOther, [](const Vector3 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector3iProperty &)>([](lua::State *l, LStringProperty &prop, LVector3iProperty &propOther) {
		prop->Link<pragma::util::Vector3iProperty, Vector3i>(*propOther, [](const Vector3i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector4Property &)>([](lua::State *l, LStringProperty &prop, LVector4Property &propOther) {
		prop->Link<pragma::util::Vector4Property, ::Vector4>(*propOther, [](const ::Vector4 &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LVector4iProperty &)>([](lua::State *l, LStringProperty &prop, LVector4iProperty &propOther) {
		prop->Link<pragma::util::Vector4iProperty, ::Vector4i>(*propOther, [](const ::Vector4i &v) -> std::string { return std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z) + " " + std::to_string(v.w); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LQuatProperty &)>([](lua::State *l, LStringProperty &prop, LQuatProperty &propOther) {
		prop->Link<pragma::util::QuatProperty, Quat>(*propOther, [](const Quat &v) -> std::string { return std::to_string(v.w) + " " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z); });
	}));
	strDef.def("Link", static_cast<void (*)(lua::State *, LStringProperty &, LEulerAnglesProperty &)>([](lua::State *l, LStringProperty &prop, LEulerAnglesProperty &propOther) {
		prop->Link<pragma::util::EulerAnglesProperty, EulerAngles>(*propOther, [](const EulerAngles &v) -> std::string { return std::to_string(v.p) + " " + std::to_string(v.y) + " " + std::to_string(v.r); });
	}));
	modUtil[strDef];

	// Matrix
	auto mat2Def = luabind::class_<LMatrix2Property, LBasePropertyWrapper>("Mat2Property");
	add_generic_methods<LMatrix2Property, ::Mat2, luabind::class_<LMatrix2Property, LBasePropertyWrapper>>(mat2Def);
	mat2Def.def(luabind::constructor<>());
	mat2Def.def(luabind::constructor<::Mat2>());
	mat2Def.def(luabind::tostring(luabind::const_self));
	mat2Def.def("Link", static_cast<void (*)(lua::State *, LMatrix2Property &, LMatrix2Property &)>(link<LMatrix2Property, ::Mat2>));
	modUtil[mat2Def];

	auto mat2x3Def = luabind::class_<LMatrix2x3Property, LBasePropertyWrapper>("Mat2x3Property");
	add_generic_methods<LMatrix2x3Property, ::Mat2x3, luabind::class_<LMatrix2x3Property, LBasePropertyWrapper>>(mat2x3Def);
	mat2x3Def.def(luabind::constructor<>());
	mat2x3Def.def(luabind::constructor<::Mat2x3>());
	mat2x3Def.def(luabind::tostring(luabind::const_self));
	mat2x3Def.def("Link", static_cast<void (*)(lua::State *, LMatrix2x3Property &, LMatrix2x3Property &)>(link<LMatrix2x3Property, ::Mat2x3>));
	modUtil[mat2x3Def];

	auto mat3x2Def = luabind::class_<LMatrix3x2Property, LBasePropertyWrapper>("Mat3x2Property");
	add_generic_methods<LMatrix3x2Property, ::Mat3x2, luabind::class_<LMatrix3x2Property, LBasePropertyWrapper>>(mat3x2Def);
	mat3x2Def.def(luabind::constructor<>());
	mat3x2Def.def(luabind::constructor<::Mat3x2>());
	mat3x2Def.def(luabind::tostring(luabind::const_self));
	mat3x2Def.def("Link", static_cast<void (*)(lua::State *, LMatrix3x2Property &, LMatrix3x2Property &)>(link<LMatrix3x2Property, ::Mat3x2>));
	modUtil[mat3x2Def];

	auto mat3Def = luabind::class_<LMatrix3Property, LBasePropertyWrapper>("Mat3Property");
	add_generic_methods<LMatrix3Property, ::Mat3, luabind::class_<LMatrix3Property, LBasePropertyWrapper>>(mat3Def);
	mat3Def.def(luabind::constructor<>());
	mat3Def.def(luabind::constructor<::Mat3>());
	mat3Def.def(luabind::tostring(luabind::const_self));
	mat3Def.def("Link", static_cast<void (*)(lua::State *, LMatrix3Property &, LMatrix3Property &)>(link<LMatrix3Property, ::Mat3>));
	modUtil[mat3Def];

	auto mat3x4Def = luabind::class_<LMatrix3x4Property, LBasePropertyWrapper>("Mat3x4Property");
	add_generic_methods<LMatrix3x4Property, ::Mat3x4, luabind::class_<LMatrix3x4Property, LBasePropertyWrapper>>(mat3x4Def);
	mat3x4Def.def(luabind::constructor<>());
	mat3x4Def.def(luabind::constructor<::Mat3x4>());
	mat3x4Def.def(luabind::tostring(luabind::const_self));
	mat3x4Def.def("Link", static_cast<void (*)(lua::State *, LMatrix3x4Property &, LMatrix3x4Property &)>(link<LMatrix3x4Property, ::Mat3x4>));
	modUtil[mat3x4Def];

	auto mat4x3Def = luabind::class_<LMatrix4x3Property, LBasePropertyWrapper>("Mat4x3Property");
	add_generic_methods<LMatrix4x3Property, ::Mat4x3, luabind::class_<LMatrix4x3Property, LBasePropertyWrapper>>(mat4x3Def);
	mat4x3Def.def(luabind::constructor<>());
	mat4x3Def.def(luabind::constructor<::Mat4x3>());
	mat4x3Def.def(luabind::tostring(luabind::const_self));
	mat4x3Def.def("Link", static_cast<void (*)(lua::State *, LMatrix4x3Property &, LMatrix4x3Property &)>(link<LMatrix4x3Property, ::Mat4x3>));
	modUtil[mat4x3Def];

	auto mat4Def = luabind::class_<LMatrix4Property, LBasePropertyWrapper>("Mat4Property");
	add_generic_methods<LMatrix4Property, ::Mat4, luabind::class_<LMatrix4Property, LBasePropertyWrapper>>(mat4Def);
	mat4Def.def(luabind::constructor<>());
	mat4Def.def(luabind::constructor<::Mat4>());
	mat4Def.def(luabind::tostring(luabind::const_self));
	mat4Def.def("Link", static_cast<void (*)(lua::State *, LMatrix4Property &, LMatrix4Property &)>(link<LMatrix4Property, ::Mat4>));
	modUtil[mat4Def];
}

void Lua::Property::push(lua::State *l, pragma::util::Int8Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::UInt8Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Int16Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::UInt16Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Int32Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::UInt32Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Int64Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::UInt64Property &prop) { push_property<LGenericIntPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::FloatProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::DoubleProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::LongDoubleProperty &prop) { push_property<LGenericFloatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::BoolProperty &prop) { push_property<LBoolPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::ColorProperty &prop) { push_property<LColorPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::EulerAnglesProperty &prop) { push_property<LEulerAnglesPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector2Property &prop) { push_property<LVector2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector2iProperty &prop) { push_property<LVector2iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector3Property &prop) { push_property<LVector3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector3iProperty &prop) { push_property<LVector3iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector4Property &prop) { push_property<LVector4PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Vector4iProperty &prop) { push_property<LVector4iPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::QuatProperty &prop) { push_property<LQuatPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::StringProperty &prop) { push_property<LStringPropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix2Property &prop) { push_property<LMatrix2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix2x3Property &prop) { push_property<LMatrix2x3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix3x2Property &prop) { push_property<LMatrix3x2PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix3Property &prop) { push_property<LMatrix3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix3x4Property &prop) { push_property<LMatrix3x4PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix4x3Property &prop) { push_property<LMatrix4x3PropertyWrapper>(l, prop); }
void Lua::Property::push(lua::State *l, pragma::util::Matrix4Property &prop) { push_property<LMatrix4PropertyWrapper>(l, prop); }

void Lua::Property::push(lua::State *l, pragma::util::BaseProperty &prop)
{
	auto typeIndex = std::type_index(typeid(prop));
	if(typeIndex == std::type_index(typeid(pragma::util::Int8Property)))
		push(l, static_cast<pragma::util::Int8Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::UInt8Property)))
		push(l, static_cast<pragma::util::UInt8Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Int16Property)))
		push(l, static_cast<pragma::util::Int16Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::UInt16Property)))
		push(l, static_cast<pragma::util::UInt16Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Int32Property)))
		push(l, static_cast<pragma::util::Int32Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::UInt32Property)))
		push(l, static_cast<pragma::util::UInt32Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Int64Property)))
		push(l, static_cast<pragma::util::Int64Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::UInt64Property)))
		push(l, static_cast<pragma::util::UInt64Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::FloatProperty)))
		push(l, static_cast<pragma::util::FloatProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::DoubleProperty)))
		push(l, static_cast<pragma::util::DoubleProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::LongDoubleProperty)))
		push(l, static_cast<pragma::util::LongDoubleProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::BoolProperty)))
		push(l, static_cast<pragma::util::BoolProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::ColorProperty)))
		push(l, static_cast<pragma::util::ColorProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::EulerAnglesProperty)))
		push(l, static_cast<pragma::util::EulerAnglesProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector2Property)))
		push(l, static_cast<pragma::util::Vector2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector2iProperty)))
		push(l, static_cast<pragma::util::Vector2iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector3Property)))
		push(l, static_cast<pragma::util::Vector3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector3iProperty)))
		push(l, static_cast<pragma::util::Vector3iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector4Property)))
		push(l, static_cast<pragma::util::Vector4Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Vector4iProperty)))
		push(l, static_cast<pragma::util::Vector4iProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::QuatProperty)))
		push(l, static_cast<pragma::util::QuatProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::StringProperty)))
		push(l, static_cast<pragma::util::StringProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::EntityProperty)))
		push(l, static_cast<pragma::EntityProperty &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix2Property)))
		push(l, static_cast<pragma::util::Matrix2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix2x3Property)))
		push(l, static_cast<pragma::util::Matrix2x3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix3x2Property)))
		push(l, static_cast<pragma::util::Matrix3x2Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix3Property)))
		push(l, static_cast<pragma::util::Matrix3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix3x4Property)))
		push(l, static_cast<pragma::util::Matrix3x4Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix4x3Property)))
		push(l, static_cast<pragma::util::Matrix4x3Property &>(prop));
	else if(typeIndex == std::type_index(typeid(pragma::util::Matrix4Property)))
		push(l, static_cast<pragma::util::Matrix4Property &>(prop));
	else
		throw std::logic_error("Unregistered property type!");
}
