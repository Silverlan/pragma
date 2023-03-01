/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_UTIL_CLASS_HPP__
#define __LUA_UTIL_CLASS_HPP__

#include <luasystem.h>
#include <sharedutils/util_string_literal.hpp>

namespace pragma {
	namespace lua {
		template<typename... Types>
		luabind::class_<Types...> register_class(const char *name)
		{
			auto def = luabind::class_<Types...>(name);
			def.def(luabind::tostring(luabind::self));
			return def;
		}
		template<typename TBase, typename... Types>
		luabind::class_<TBase, Types...> register_class(const char *name, void (*tostring)(const TBase &))
		{
			auto def = luabind::class_<Types...>(name);
			def.def("__tostring", tostring);
			return def;
		}
		template<util::StringLiteral TStr, typename... Types>
		luabind::class_<Types...> register_class(const char *name)
		{
			auto def = luabind::class_<Types...>(name);
			def.def(
			  "__tostring", +[]() -> const char * { return TStr; });
			return def;
		}
		template<util::StringLiteral TStr, typename... Types>
		luabind::class_<Types...> register_class()
		{
			auto def = luabind::class_<Types...>(TStr.value);
			def.def(
			  "__tostring", +[]() -> const char * { return TStr.value; });
			return def;
		}
	};
};

#endif
