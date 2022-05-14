/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUDM_HPP__
#define __LUDM_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/types/udm.hpp"
#include <udm_types.hpp>

namespace udm {struct Array;};
namespace Lua
{
	class Interface;
	namespace udm
	{
		DLLNETWORK void register_library(Lua::Interface &lua);
		DLLNETWORK void set_array_values(lua_State *l,::udm::Array &a,luabind::tableT<void> t,uint32_t tIdx);
		DLLNETWORK bool set_array_value(lua_State *l,::udm::Array &a,int32_t idx,const luabind::object &o);
		DLLNETWORK void table_to_udm(const Lua::tb<void> &t,::udm::LinkedPropertyWrapper &udm);
		DLLNETWORK luabind::object udm_to_value(lua_State *l,::udm::LinkedPropertyWrapperArg udm);
		template<typename T>
			void lerp_value(const T &value0,const T &value1,float f,T &outValue,::udm::Type type);
		template<typename T>
			T cast_object(const Lua::udm_ng &value)
		{
			luabind::object o{value};
			T v;
			auto type = static_cast<Lua::Type>(luabind::type(o));
			switch(type)
			{
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Lua::Type::Bool:
				if constexpr(::udm::is_convertible<bool,T>())
					v = ::udm::convert<bool,T>(luabind::object_cast<bool>(o));
				else
					v = luabind::object_cast<T>(o);
				break;
			case Lua::Type::Number:
				if constexpr(::udm::is_convertible<double,T>())
					v = ::udm::convert<double,T>(luabind::object_cast<double>(o));
				else
					v = luabind::object_cast<T>(o);
				break;
			default:
				v = luabind::object_cast<T>(o);
				break;
			}
			return v;
		}
		template<typename T>
			T cast_object_nothrow(const Lua::udm_ng &value)
		{
			luabind::object o{value};
			T v;
			auto type = static_cast<Lua::Type>(luabind::type(o));
			switch(type)
			{
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Lua::Type::Bool:
				if constexpr(::udm::is_convertible<bool,T>())
					v = ::udm::convert<bool,T>(luabind::object_cast_nothrow<bool>(o,false));
				else
					v = luabind::object_cast_nothrow<T>(o,T{});
				break;
			case Lua::Type::Number:
				if constexpr(::udm::is_convertible<double,T>())
					v = ::udm::convert<double,T>(luabind::object_cast_nothrow<double>(o,0.0));
				else
					v = luabind::object_cast_nothrow<T>(o,T{});
				break;
			default:
				v = luabind::object_cast_nothrow<T>(o,T{});
				break;
			}
			return v;
		}
	};
};

template<typename T>
	void Lua::udm::lerp_value(const T &value0,const T &value1,float f,T &outValue,::udm::Type type)
{
	using TBase = ::udm::base_type<T>;
	if constexpr(std::is_same_v<TBase,::udm::Transform> || std::is_same_v<TBase,::udm::ScaledTransform>)
	{
		outValue.SetOrigin(uvec::lerp(value0.GetOrigin(),value1.GetOrigin(),f));
		outValue.SetRotation(uquat::slerp(value0.GetRotation(),value1.GetRotation(),f));
		if constexpr(std::is_same_v<TBase,::udm::ScaledTransform>)
			outValue.SetScale(uvec::lerp(value0.GetScale(),value1.GetScale(),f));
	}
	else if constexpr(std::is_same_v<TBase,::udm::Half>)
		outValue = static_cast<float>(umath::lerp(static_cast<float>(value0),static_cast<float>(value1),f));
	else if constexpr(::udm::is_arithmetic<TBase>)
		outValue = umath::lerp(value0,value1,f);
	else if constexpr(::udm::is_vector_type<TBase>)
	{
		if constexpr(std::is_integral_v<typename TBase::value_type>)
			; // TODO
		else
			outValue = value0 +(value1 -value0) *f;
	}
	else if constexpr(std::is_same_v<TBase,::udm::EulerAngles>)
	{
		auto q0 = uquat::create(value0);
		auto q1 = uquat::create(value1);
		auto qr = uquat::slerp(q0,q1,f);
		outValue = EulerAngles{qr};
	}
	else if constexpr(std::is_same_v<TBase,::udm::Quaternion>)
		outValue = uquat::slerp(value0,value1,f);
	else
	{
		outValue = value0;
		auto n = ::udm::get_numeric_component_count(type);
		for(auto i=decltype(n){0u};i<n;++i)
		{
			auto &f0 = *(reinterpret_cast<const float*>(&value0) +i);
			auto &f1 = *(reinterpret_cast<const float*>(&value1) +i);

			*(reinterpret_cast<float*>(&outValue) +i) = umath::lerp(f0,f1,f);
		}
	}
}

#endif
