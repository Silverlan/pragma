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

namespace udm {
	struct Array;
};
namespace Lua {
	class Interface;
	namespace udm {
		class DLLNETWORK LuaUdmArrayIterator {
		  public:
			LuaUdmArrayIterator(::udm::PropertyWrapper &prop);

			::udm::ArrayIterator<::udm::LinkedPropertyWrapper> begin() { return m_property->begin(); }
			::udm::ArrayIterator<::udm::LinkedPropertyWrapper> end() { return m_property->end(); }
		  private:
			::udm::PropertyWrapper *m_property = nullptr;
		};

		DLLNETWORK void register_library(Lua::Interface &lua);
		DLLNETWORK void set_array_values(lua_State *l, ::udm::Array &a, luabind::tableT<void> t, uint32_t tIdx);
		DLLNETWORK bool set_array_value(lua_State *l, ::udm::Array &a, int32_t idx, const luabind::object &o);
		DLLNETWORK void table_to_udm(const Lua::tb<void> &t, ::udm::LinkedPropertyWrapper &udm);
		DLLNETWORK ::udm::Type determine_udm_type(const luabind::object &val);
		DLLNETWORK luabind::object udm_to_value(lua_State *l, ::udm::LinkedPropertyWrapperArg udm);
		template<typename T>
		T cast_object(const Lua::udm_ng &value)
		{
			luabind::object o {value};
			T v;
			auto type = static_cast<Lua::Type>(luabind::type(o));
			switch(type) {
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Lua::Type::Bool:
				if constexpr(::udm::is_convertible<bool, T>())
					v = ::udm::convert<bool, T>(luabind::object_cast<bool>(o));
				else
					v = luabind::object_cast<T>(o);
				break;
			case Lua::Type::Number:
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
		T cast_object_nothrow(const Lua::udm_ng &value)
		{
			luabind::object o {value};
			T v;
			auto type = static_cast<Lua::Type>(luabind::type(o));
			switch(type) {
			// Booleans are distinct types in Lua, but we want to treat them as a numeric type (as well as the other way around)
			case Lua::Type::Bool:
				if constexpr(::udm::is_convertible<bool, T>())
					v = ::udm::convert<bool, T>(luabind::object_cast_nothrow<bool>(o, false));
				else
					v = luabind::object_cast_nothrow<T>(o, T {});
				break;
			case Lua::Type::Number:
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

#endif
