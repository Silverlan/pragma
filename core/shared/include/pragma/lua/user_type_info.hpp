/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_USER_TYPE_INFO_HPP__
#define __LUA_USER_TYPE_INFO_HPP__

#include <luabind/detail/signature_types.hpp>
#include <sharedutils/util.h>

#define LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER_C(TYPE, IDENTIFIER, CUSTOM)                                                                                                                                                                                                                          \
	namespace luabind::detail {                                                                                                                                                                                                                                                                  \
		template<class T>                                                                                                                                                                                                                                                                        \
		    requires(util::is_specialization<base_type<T>, TYPE>::value)                                                                                                                                                                                                                         \
		struct get_user_type_info<T> {                                                                                                                                                                                                                                                           \
			static void get(lua_State *L, TypeInfo &outTypeInfo)                                                                                                                                                                                                                                 \
			{                                                                                                                                                                                                                                                                                    \
				outTypeInfo.typeIdentifier = IDENTIFIER;                                                                                                                                                                                                                                         \
				CUSTOM                                                                                                                                                                                                                                                                           \
			}                                                                                                                                                                                                                                                                                    \
		};                                                                                                                                                                                                                                                                                       \
	};

#define LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(TYPE, IDENTIFIER) LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER_C(TYPE, IDENTIFIER, {});

LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER_C(luabind::optional, "optional", { outTypeInfo.isOptional = true; });
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::userData, "userData");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::classObject, "classObject");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::map, "map");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::tableT, "tableT");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::tableTT, "tableTT");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::variant, "variant");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::variadic, "variadic");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::mult, "mult");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::typehint, "typehint");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(luabind::functype, "functype");

LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER_C(std::optional, "optional", { outTypeInfo.isOptional = true; });
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::pair, "pair");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::tuple, "tuple");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::vector, "vector");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::shared_ptr, "shared_ptr");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::unique_ptr, "unique_ptr");
LUA_DEFINE_TEMPLATE_TYPE_IDENTIFIER(std::unordered_map, "map");

namespace luabind::detail {
	template<class T>
	    requires(util::is_specialization_array<base_type<T>>::value)
	struct get_user_type_info<T> {
		static void get(lua_State *L, TypeInfo &outTypeInfo) { outTypeInfo.typeIdentifier = "array"; }
	};
};

#endif
