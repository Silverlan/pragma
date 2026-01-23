// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.class_wrapper;

export import pragma.lua;
import pragma.util;

export namespace pragma {
	namespace LuaCore {
		namespace detail {
			DLLNETWORK std::string tostring(const luabind::object &o);
			DLLNETWORK void register_lua_debug_tostring(lua::State *l, const std::type_info &typeInfo);
		};

		template<typename... Types>
		struct ClassWrapper {
			ClassWrapper(lua::State *l, const char *name, const std::type_info &typeInfo) : l {l}, luaClass {name}, typeInfo {typeInfo} {}
			~ClassWrapper() { detail::register_lua_debug_tostring(l, typeInfo); }
			lua::State *l;
			luabind::class_<Types...> luaClass;
			const std::type_info &typeInfo;
			luabind::class_<Types...> &operator*() { return luaClass; }
			luabind::class_<Types...> *operator->() { return &luaClass; }
		};

		template<typename... Types>
		ClassWrapper<Types...> register_class(lua::State *l, const char *name)
		{
			using FirstType = std::tuple_element_t<0, std::tuple<Types...>>;
			ClassWrapper<Types...> def {l, name, typeid(FirstType)};
			def->def(luabind::tostring(luabind::self));
			return def;
		}
		template<typename TBase, typename... Types>
		ClassWrapper<TBase, Types...> register_class(lua::State *l, const char *name, void (*tostring)(lua::State*, const TBase &))
		{
			ClassWrapper<TBase, Types...> def {l, name, typeid(TBase)};
			def->def("__tostring", tostring);
			return def;
		}
		template<util::StringLiteral TStr, typename... Types>
		ClassWrapper<Types...> register_class(lua::State *l, const char *name)
		{
			using FirstType = std::tuple_element_t<0, std::tuple<Types...>>;
			ClassWrapper<Types...> def = {l, name, typeid(FirstType)};
			def->def("__tostring", +[]() -> const char * { return TStr; });
			return def;
		}
		template<util::StringLiteral TStr, typename... Types>
		ClassWrapper<Types...> register_class(lua::State *l)
		{
			using FirstType = std::tuple_element_t<0, std::tuple<Types...>>;
			ClassWrapper<Types...> def {l, TStr.value, typeid(FirstType)};
			def->def("__tostring", +[]() -> const char * { return TStr.value; });
			return def;
		}
	};
};
