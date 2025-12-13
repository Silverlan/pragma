// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.udm;

export import :scripting.lua.api;
export import :scripting.lua.types;
export import pragma.lua;
export import pragma.udm;

export namespace Lua {
	namespace udm {
		class DLLNETWORK LuaUdmArrayIterator {
		  public:
			LuaUdmArrayIterator(::udm::PropertyWrapper &prop);

			::udm::ArrayIterator<::udm::LinkedPropertyWrapper> begin() { return m_property->begin(); }
			::udm::ArrayIterator<::udm::LinkedPropertyWrapper> end() { return m_property->end(); }
		  private:
			::udm::PropertyWrapper *m_property = nullptr;
		};

		DLLNETWORK void register_library(Interface &lua);
		DLLNETWORK void set_array_values(lua::State *l, ::udm::Array &a, luabind::tableT<void> t, uint32_t tIdx);
		DLLNETWORK bool set_array_value(lua::State *l, ::udm::Array &a, int32_t idx, const luabind::object &o);
		DLLNETWORK void table_to_udm(const tb<void> &t, ::udm::LinkedPropertyWrapper &udm);
		DLLNETWORK ::udm::Type determine_udm_type(const luabind::object &val);
		DLLNETWORK luabind::object udm_to_value(lua::State *l, ::udm::LinkedPropertyWrapperArg udm);
		template<typename T>
		T cast_object(const udm_ng &value)
		{
			luabind::object o {value};
			T v;
			auto type = static_cast<Type>(luabind::type(o));
			switch(type) {
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Type::Bool:
				if constexpr(::udm::is_convertible<bool, T>())
					v = ::udm::convert<bool, T>(luabind::object_cast<bool>(o));
				else
					v = luabind::object_cast<T>(o);
				break;
			case Type::Number:
				if constexpr(::udm::is_convertible<double, T>())
					v = ::udm::convert<double, T>(luabind::object_cast<double>(o));
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
		T cast_object_nothrow(const udm_ng &value)
		{
			luabind::object o {value};
			T v;
			auto type = static_cast<Type>(luabind::type(o));
			switch(type) {
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Type::Bool:
				if constexpr(::udm::is_convertible<bool, T>())
					v = ::udm::convert<bool, T>(luabind::object_cast_nothrow<bool>(o, false));
				else
					v = luabind::object_cast_nothrow<T>(o, T {});
				break;
			case Type::Number:
				if constexpr(::udm::is_convertible<double, T>())
					v = ::udm::convert<double, T>(luabind::object_cast_nothrow<double>(o, 0.0));
				else
					v = luabind::object_cast_nothrow<T>(o, T {});
				break;
			default:
				v = luabind::object_cast_nothrow<T>(o, T {});
				break;
			}
			return v;
		}
	};
};
