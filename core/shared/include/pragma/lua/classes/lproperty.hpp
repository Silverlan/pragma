/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LPROPERTY_HPP__
#define __LPROPERTY_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/entities/entity_property.hpp"
#include "pragma/lua/classes/lproperty_entity.hpp"
#include "pragma/lua/lua_call.hpp"
#include <any>
#include <sharedutils/property/util_property.hpp>
#include <sharedutils/property/util_property_color.hpp>
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_quat.hpp>
#include <sharedutils/property/util_property_matrix.hpp>
#include <sharedutils/property/util_property_euler_angles.hpp>

#define DEFINE_LUA_NUMBER_PROPERTY(TYPE,UNDERLYING_TYPE) \
	using L##TYPE##Property = TLNumberPropertyWrapper<util::TYPE##Property,UNDERLYING_TYPE>; \
	static UNDERLYING_TYPE operator+(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v +prop->GetValue();} \
	static UNDERLYING_TYPE operator-(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v -prop->GetValue();} \
	static UNDERLYING_TYPE operator*(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v *prop->GetValue();} \
	static UNDERLYING_TYPE operator/(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v /prop->GetValue();} \
	static bool operator<(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v < *prop;} \
	static bool operator<=(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v <= *prop;} \
	static bool operator==(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return **prop == v;} \
	static std::ostream& operator<<(std::ostream &str,const L##TYPE##Property &v) {return str<<**v;}

DEFINE_LUA_NUMBER_PROPERTY(Int8,int8_t);
DEFINE_LUA_NUMBER_PROPERTY(UInt8,uint8_t);
DEFINE_LUA_NUMBER_PROPERTY(Int16,int16_t);
DEFINE_LUA_NUMBER_PROPERTY(UInt16,uint16_t);
DEFINE_LUA_NUMBER_PROPERTY(Int32,int32_t);
DEFINE_LUA_NUMBER_PROPERTY(UInt32,uint32_t);
DEFINE_LUA_NUMBER_PROPERTY(Int64,int64_t);
DEFINE_LUA_NUMBER_PROPERTY(UInt64,uint64_t);

DEFINE_LUA_NUMBER_PROPERTY(Float,float);
DEFINE_LUA_NUMBER_PROPERTY(Double,double);
DEFINE_LUA_NUMBER_PROPERTY(LongDouble,long double);

//////////

// Bool
class LBoolPropertyWrapper
	: public LSimplePropertyWrapper<util::BoolProperty,bool>
{
public:
	using LSimplePropertyWrapper<util::BoolProperty,bool>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<util::BoolProperty,bool>::operator*;
	using LSimplePropertyWrapper<util::BoolProperty,bool>::operator->;
	virtual util::BoolProperty &GetProperty() const override {return *static_cast<util::BoolProperty*>(prop.get());}
};
using LBoolProperty = LBoolPropertyWrapper;

// Color
class LColorPropertyWrapper
	: public LSimplePropertyWrapper<util::ColorProperty,Color>
{
public:
	using LSimplePropertyWrapper<util::ColorProperty,Color>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<util::ColorProperty,Color>::operator*;
	using LSimplePropertyWrapper<util::ColorProperty,Color>::operator->;
	virtual util::ColorProperty &GetProperty() const override {return *static_cast<util::ColorProperty*>(prop.get());}

	LColorPropertyWrapper()
		: LSimplePropertyWrapper()
	{}
	LColorPropertyWrapper(const Color &col)
		: LSimplePropertyWrapper(col)
	{}
	LColorPropertyWrapper(Int16 r,Int16 g,Int16 b,Int16 a)
		: LSimplePropertyWrapper(Color{r,g,b,a})
	{}
	LColorPropertyWrapper(const std::string &str)
		: LSimplePropertyWrapper(Color{str})
	{}
	LColorPropertyWrapper(const Vector3 &v)
		: LSimplePropertyWrapper(Color{v})
	{}
	LColorPropertyWrapper(const Vector4 &v)
		: LSimplePropertyWrapper(Color{v})
	{}
	LColorPropertyWrapper operator/(float f)
	{
		GetProperty() /= f;
		return *this;
	}
	LColorPropertyWrapper operator*(float f)
	{
		GetProperty() *= f;
		return *this;
	}
	LColorPropertyWrapper operator+(const Color &other)
	{
		GetProperty() += other;
		return *this;
	}
	LColorPropertyWrapper operator+(const LColorPropertyWrapper &propOther)
	{
		GetProperty() += *propOther;
		return *this;
	}
	LColorPropertyWrapper operator-(const Color &other)
	{
		GetProperty() -= other;
		return *this;
	}
	LColorPropertyWrapper operator-(const LColorPropertyWrapper &propOther)
	{
		GetProperty() -= *propOther;
		return *this;
	}
};
using LColorProperty = LColorPropertyWrapper;

// Euler Angles
class LEulerAnglesPropertyWrapper
	: public LSimplePropertyWrapper<util::EulerAnglesProperty,EulerAngles>
{
public:
	using LSimplePropertyWrapper<util::EulerAnglesProperty,EulerAngles>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<util::EulerAnglesProperty,EulerAngles>::operator*;
	using LSimplePropertyWrapper<util::EulerAnglesProperty,EulerAngles>::operator->;
	virtual util::EulerAnglesProperty &GetProperty() const override {return *static_cast<util::EulerAnglesProperty*>(prop.get());}

	LEulerAnglesPropertyWrapper()
		: LSimplePropertyWrapper()
	{}
	LEulerAnglesPropertyWrapper(const EulerAngles &col)
		: LSimplePropertyWrapper(col)
	{}
	LEulerAnglesPropertyWrapper(float p,float y,float r)
		: LSimplePropertyWrapper(EulerAngles{p,y,r})
	{}
	LEulerAnglesPropertyWrapper(const std::string &str)
		: LSimplePropertyWrapper(EulerAngles{str})
	{}
	LEulerAnglesPropertyWrapper operator/(float f)
	{
		GetProperty() /= f;
		return *this;
	}
	LEulerAnglesPropertyWrapper operator*(float f)
	{
		GetProperty() *= f;
		return *this;
	}
	LEulerAnglesPropertyWrapper operator+(const EulerAngles &other)
	{
		GetProperty() += other;
		return *this;
	}
	LEulerAnglesPropertyWrapper operator+(const LEulerAnglesPropertyWrapper &propOther)
	{
		GetProperty() += *propOther;
		return *this;
	}
	LEulerAnglesPropertyWrapper operator-(const EulerAngles &other)
	{
		GetProperty() -= other;
		return *this;
	}
	LEulerAnglesPropertyWrapper operator-(const LEulerAnglesPropertyWrapper &propOther)
	{
		GetProperty() -= *propOther;
		return *this;
	}
};
using LEulerAnglesProperty = LEulerAnglesPropertyWrapper;

template<class TProperty,typename T>
	class TLVectorPropertyWrapper
		: public LSimplePropertyWrapper<TProperty,T>
{
public:
	using LSimplePropertyWrapper<TProperty,T>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<TProperty,T>::operator*;
	using LSimplePropertyWrapper<TProperty,T>::operator->;
	virtual TProperty &GetProperty() const override {return *static_cast<TProperty*>(this->prop.get());}
	TLVectorPropertyWrapper<TProperty,T> operator/(float f)
	{
		GetProperty() /= f;
		return *this;
	}
	TLVectorPropertyWrapper<TProperty,T> operator*(float f)
	{
		GetProperty() /= f;
		return *this;
	}
	TLVectorPropertyWrapper<TProperty,T> operator+(const T &other)
	{
		GetProperty() += other;
		return *this;
	}
	TLVectorPropertyWrapper<TProperty,T> operator+(const TLVectorPropertyWrapper<TProperty,T> &propOther)
	{
		GetProperty() += *propOther;
		return *this;
	}
	TLVectorPropertyWrapper<TProperty,T> operator-(const T &other)
	{
		GetProperty() -= other;
		return *this;
	}
	TLVectorPropertyWrapper<TProperty,T> operator-(const TLVectorPropertyWrapper<TProperty,T> &propOther)
	{
		GetProperty() -= *propOther;
		return *this;
	}
};

class LVector2PropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector2Property,Vector2>
{
public:
	using TLVectorPropertyWrapper<util::Vector2Property,Vector2>::TLVectorPropertyWrapper;
	LVector2PropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector2Property,Vector2>()
	{}
	LVector2PropertyWrapper(const Vector2 &v)
		: TLVectorPropertyWrapper<util::Vector2Property,Vector2>(v)
	{}
	LVector2PropertyWrapper(float x,float y)
		: TLVectorPropertyWrapper<util::Vector2Property,Vector2>(Vector2{x,y})
	{}
};

class LVector2iPropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector2iProperty,Vector2i>
{
public:
	using TLVectorPropertyWrapper<util::Vector2iProperty,Vector2i>::TLVectorPropertyWrapper;
	LVector2iPropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector2iProperty,Vector2i>()
	{}
	LVector2iPropertyWrapper(const Vector2i &v)
		: TLVectorPropertyWrapper<util::Vector2iProperty,Vector2i>(v)
	{}
	LVector2iPropertyWrapper(int32_t x,int32_t y)
		: TLVectorPropertyWrapper<util::Vector2iProperty,Vector2i>(Vector2i{x,y})
	{}
};

class LVector3PropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector3Property,Vector3>
{
public:
	using TLVectorPropertyWrapper<util::Vector3Property,Vector3>::TLVectorPropertyWrapper;
	LVector3PropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector3Property,Vector3>()
	{}
	LVector3PropertyWrapper(const Vector3 &v)
		: TLVectorPropertyWrapper<util::Vector3Property,Vector3>(v)
	{}
	LVector3PropertyWrapper(float x,float y,float z)
		: TLVectorPropertyWrapper<util::Vector3Property,Vector3>(Vector3{x,y,z})
	{}
};

class LVector3iPropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector3iProperty,Vector3i>
{
public:
	using TLVectorPropertyWrapper<util::Vector3iProperty,Vector3i>::TLVectorPropertyWrapper;
	LVector3iPropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector3iProperty,Vector3i>()
	{}
	LVector3iPropertyWrapper(const Vector3i &v)
		: TLVectorPropertyWrapper<util::Vector3iProperty,Vector3i>(v)
	{}
	LVector3iPropertyWrapper(int32_t x,int32_t y,int32_t z)
		: TLVectorPropertyWrapper<util::Vector3iProperty,Vector3i>(Vector3i{x,y,z})
	{}
};

class LVector4PropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector4Property,Vector4>
{
public:
	using TLVectorPropertyWrapper<util::Vector4Property,Vector4>::TLVectorPropertyWrapper;
	LVector4PropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector4Property,Vector4>()
	{}
	LVector4PropertyWrapper(const Vector4 &v)
		: TLVectorPropertyWrapper<util::Vector4Property,Vector4>(v)
	{}
	LVector4PropertyWrapper(float x,float y,float z,float w)
		: TLVectorPropertyWrapper<util::Vector4Property,Vector4>(Vector4{x,y,z,w})
	{}
};

class LVector4iPropertyWrapper
	: public TLVectorPropertyWrapper<util::Vector4iProperty,Vector4i>
{
public:
	using TLVectorPropertyWrapper<util::Vector4iProperty,Vector4i>::TLVectorPropertyWrapper;
	LVector4iPropertyWrapper()
		: TLVectorPropertyWrapper<util::Vector4iProperty,Vector4i>()
	{}
	LVector4iPropertyWrapper(const Vector4i &v)
		: TLVectorPropertyWrapper<util::Vector4iProperty,Vector4i>(v)
	{}
	LVector4iPropertyWrapper(int32_t x,int32_t y,int32_t z,int32_t w)
		: TLVectorPropertyWrapper<util::Vector4iProperty,Vector4i>(Vector4i{x,y,z,w})
	{}
};

#define DEFINE_LUA_VECTOR_PROPERTY(TYPE,UNDERLYING_PROP_TYPE,UNDERLYING_TYPE) \
	using L##TYPE##Property = L##TYPE##PropertyWrapper; \
	static UNDERLYING_PROP_TYPE operator+(const UNDERLYING_PROP_TYPE &v,const L##TYPE##Property &prop) {return v +prop->GetValue();} \
	static UNDERLYING_PROP_TYPE operator-(const UNDERLYING_PROP_TYPE &v,const L##TYPE##Property &prop) {return v -prop->GetValue();} \
	static UNDERLYING_PROP_TYPE operator*(const UNDERLYING_PROP_TYPE &v,const L##TYPE##Property &prop) {return v *prop->GetValue();} \
	static UNDERLYING_PROP_TYPE operator/(const UNDERLYING_PROP_TYPE &v,const L##TYPE##Property &prop) {return v /prop->GetValue();} \
	static UNDERLYING_PROP_TYPE operator*(UNDERLYING_TYPE v,const L##TYPE##Property &prop) {return v *prop->GetValue();} \
	static bool operator==(const UNDERLYING_PROP_TYPE &v,const L##TYPE##Property &prop) {return **prop == v;} \
	static std::ostream& operator<<(std::ostream &str,const L##TYPE##Property &v) {return str<<**v;}

DEFINE_LUA_VECTOR_PROPERTY(Vector2,Vector2,float);
DEFINE_LUA_VECTOR_PROPERTY(Vector2i,Vector2i,int32_t);
DEFINE_LUA_VECTOR_PROPERTY(Vector3,Vector3,float);
DEFINE_LUA_VECTOR_PROPERTY(Vector3i,Vector3i,int32_t);
DEFINE_LUA_VECTOR_PROPERTY(Vector4,Vector4,float);
DEFINE_LUA_VECTOR_PROPERTY(Vector4i,Vector4i,int32_t);

// Quaternion
class LQuatPropertyWrapper
	: public LSimplePropertyWrapper<util::QuatProperty,Quat>
{
public:
	using LSimplePropertyWrapper<util::QuatProperty,Quat>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<util::QuatProperty,Quat>::operator*;
	using LSimplePropertyWrapper<util::QuatProperty,Quat>::operator->;
	virtual util::QuatProperty &GetProperty() const override {return *static_cast<util::QuatProperty*>(this->prop.get());}

	LQuatPropertyWrapper()
		: LSimplePropertyWrapper<util::QuatProperty,Quat>(uquat::identity())
	{}
	LQuatPropertyWrapper(const Quat &v)
		: LSimplePropertyWrapper<util::QuatProperty,Quat>(v)
	{}
	LQuatPropertyWrapper(float w,float x,float y,float z)
		: LSimplePropertyWrapper<util::QuatProperty,Quat>(Quat{w,x,y,z})
	{}
	LQuatPropertyWrapper(const std::string &str)
		: LSimplePropertyWrapper<util::QuatProperty,Quat>(uquat::create(str))
	{}
	LQuatPropertyWrapper operator/(float f)
	{
		GetProperty() /= f;
		return *this;
	}
	LQuatPropertyWrapper operator*(float f)
	{
		GetProperty() *= f;
		return *this;
	}
	LQuatPropertyWrapper operator*(const Quat &other)
	{
		GetProperty() *= other;
		return *this;
	}
	LQuatPropertyWrapper operator*(const LQuatPropertyWrapper &propOther)
	{
		GetProperty() *= *propOther;
		return *this;
	}
};
using LQuatProperty = LQuatPropertyWrapper;

// String
class LStringPropertyWrapper
	: public LSimplePropertyWrapper<util::StringProperty,std::string>
{
public:
	using LSimplePropertyWrapper<util::StringProperty,std::string>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<util::StringProperty,std::string>::operator*;
	using LSimplePropertyWrapper<util::StringProperty,std::string>::operator->;
	virtual util::StringProperty &GetProperty() const override {return *static_cast<util::StringProperty*>(this->prop.get());}

	LStringPropertyWrapper()
		: LSimplePropertyWrapper<util::StringProperty,std::string>()
	{}
	LStringPropertyWrapper(const std::string &v)
		: LSimplePropertyWrapper<util::StringProperty,std::string>(v)
	{}
};
using LStringProperty = LStringPropertyWrapper;

// Matrix
template<class TProperty,typename T>
	class TLMatrixPropertyWrapper
		: public LSimplePropertyWrapper<TProperty,T>
{
public:
	using LSimplePropertyWrapper<TProperty,T>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<TProperty,T>::operator*;
	using LSimplePropertyWrapper<TProperty,T>::operator->;
	virtual TProperty &GetProperty() const override {return *static_cast<TProperty*>(this->prop.get());}

	TLMatrixPropertyWrapper()
		: LSimplePropertyWrapper<TProperty,T>()
	{}
	TLMatrixPropertyWrapper(const T &v)
		: LSimplePropertyWrapper<TProperty,T>(v)
	{}
};
using LMatrix2PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix2Property,Mat2>;
using LMatrix2x3PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix2x3Property,Mat2x3>;
using LMatrix3x2PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix3x2Property,Mat3x2>;
using LMatrix3PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix3Property,Mat3>;
using LMatrix3x4PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix3x4Property,Mat3x4>;
using LMatrix4x3PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix4x3Property,Mat4x3>;
using LMatrix4PropertyWrapper = TLMatrixPropertyWrapper<util::Matrix4Property,Mat4>;
using LMatrix2Property = LMatrix2PropertyWrapper;
using LMatrix2x3Property = LMatrix2x3PropertyWrapper;
using LMatrix3x2Property = LMatrix3x2PropertyWrapper;
using LMatrix3Property = LMatrix3PropertyWrapper;
using LMatrix3x4Property = LMatrix3x4PropertyWrapper;
using LMatrix4x3Property = LMatrix4x3PropertyWrapper;
using LMatrix4Property = LMatrix4PropertyWrapper;

namespace util
{
	enum class VarType : uint8_t;
};
namespace Lua
{
	namespace Property
	{
		DLLNETWORK void register_classes(Lua::Interface &l);

		DLLNETWORK void push(lua_State *l,::util::Int8Property &prop);
		DLLNETWORK void push(lua_State *l,::util::UInt8Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Int16Property &prop);
		DLLNETWORK void push(lua_State *l,::util::UInt16Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Int32Property &prop);
		DLLNETWORK void push(lua_State *l,::util::UInt32Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Int64Property &prop);
		DLLNETWORK void push(lua_State *l,::util::UInt64Property &prop);
		DLLNETWORK void push(lua_State *l,::util::FloatProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::DoubleProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::LongDoubleProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::BoolProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::ColorProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::EulerAnglesProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector2Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector2iProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector3Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector3iProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector4Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Vector4iProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::QuatProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::StringProperty &prop);
		DLLNETWORK void push(lua_State *l,pragma::EntityProperty &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix2Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix2x3Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix3x2Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix3Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix3x4Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix4x3Property &prop);
		DLLNETWORK void push(lua_State *l,::util::Matrix4Property &prop);
		DLLNETWORK void push(lua_State *l,::util::BaseProperty &prop);

		template<typename TEnum>
			void push(lua_State *l,::util::TEnumProperty<TEnum> &prop)
		{
			Lua::Push<LGenericIntPropertyWrapper>(l,LGenericIntPropertyWrapper(std::static_pointer_cast<::util::TEnumProperty<TEnum>>(prop.shared_from_this())));
		}
		template<class TProperty,typename T>
			void add_callback(lua_State *l,TProperty &prop,const luabind::object &oCallback)
		{
			Lua::CheckFunction(l,2);
			auto cb = prop->AddCallback([l,oCallback](std::reference_wrapper<const T> oldVal,std::reference_wrapper<const T> newVal) {
				auto c = Lua::CallFunction(l,[&oCallback,&oldVal,&newVal](lua_State *l) -> Lua::StatusCode {
					oCallback.push(l);
					Lua::Push<T>(l,oldVal.get());
					Lua::Push<T>(l,newVal.get());
					return Lua::StatusCode::Ok;
				},0);
			});
			Lua::Push<CallbackHandle>(l,cb);
		}

		template<class TProperty,typename T>
			void add_modifier(lua_State *l,TProperty &prop,const luabind::object &oCallback)
		{
			Lua::CheckFunction(l,2);
			auto cb = prop->AddModifier([l,oCallback](T &val) {
				auto c = Lua::CallFunction(l,[&oCallback,&val](lua_State *l) -> Lua::StatusCode {
					oCallback.push(l);
					Lua::Push<T>(l,val);
					return Lua::StatusCode::Ok;
				},1);
				if(c == Lua::StatusCode::Ok && Lua::IsSet(l,-1))
					val = Lua::Check<T>(l,-1);
			});
			Lua::Push<CallbackHandle>(l,cb);
		}

		template<class TProperty,typename T>
			void unlink(lua_State *l,TProperty &prop)
		{
			prop->Unlink();
		}

		template<class TProperty,typename T>
			void get(lua_State *l,TProperty &prop)
		{
			Lua::Push<T>(l,prop->GetValue());
		}

		template<class TProperty,typename T>
			void set(lua_State *l,TProperty &prop,const T &val)
		{
			prop->operator=(val);
		}

		template<class TProperty,typename T>
			void set_locked(lua_State *l,TProperty &prop,bool bLocked)
		{
			prop->SetLocked(bLocked);
		}

		template<class TProperty,typename T>
			void is_locked(lua_State *l,TProperty &prop)
		{
			Lua::PushBool(l,prop->IsLocked());
		}

		template<class TProperty,typename T>
			void invoke_callbacks(lua_State *l,TProperty &prop)
		{
			prop->InvokeCallbacks();
		}
		template<class TProperty,typename T,class TLuaClass>
			void add_generic_methods(TLuaClass &classDef)
		{
			classDef.def("AddCallback",static_cast<void(*)(lua_State*,TProperty&,const luabind::object&)>(add_callback<TProperty,T>));
			classDef.def("AddModifier",static_cast<void(*)(lua_State*,TProperty&,const luabind::object&)>(add_modifier<TProperty,T>));
			classDef.def("Unlink",static_cast<void(*)(lua_State*,TProperty&)>(unlink<TProperty,T>));
			classDef.def("Get",static_cast<void(*)(lua_State*,TProperty&)>(get<TProperty,T>));
			classDef.def("Set",static_cast<void(*)(lua_State*,TProperty&,const T&)>(set<TProperty,T>));
			classDef.def("SetLocked",static_cast<void(*)(lua_State*,TProperty&,bool)>(set_locked<TProperty,T>));
			classDef.def("IsLocked",static_cast<void(*)(lua_State*,TProperty&)>(is_locked<TProperty,T>));
			classDef.def("InvokeCallbacks",static_cast<void(*)(lua_State*,TProperty&)>(invoke_callbacks<TProperty,T>));
		}
		template<class TProperty,typename T,class TLinkProperty>
			void link_different(lua_State *l,TProperty &prop,TLinkProperty &propOther)
		{
			prop->Link(*propOther);
		}

		template<class TProperty,typename T>
			void link(lua_State *l,TProperty &prop,TProperty &propOther)
		{
			link_different<TProperty,T,TProperty>(l,prop,propOther);
		}
		template<class TWrapper,class TProperty>
			void push_property(lua_State *l,TProperty &prop)
		{
			Lua::Push<TWrapper>(l,TWrapper(std::static_pointer_cast<TProperty>(prop.shared_from_this())));
		}
	};
};

#endif
