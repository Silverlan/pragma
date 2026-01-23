// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.util_entity_component;

export import :entities.components.base;

export namespace pragma {
	namespace LuaCore {
		template<typename... Types>
		luabind::class_<Types...> create_entity_component_class(const char *name)
		{
			using T = typename std::tuple_element<0, std::tuple<Types...>>::type;
			auto def = luabind::class_<Types...>(name);
			def.def(
			  "__tostring", +[](T &c) -> std::string {
				  std::stringstream ss;
				  ss << c;
				  return ss.str();
			  });
			// msvc is unable to find the operator<< overload in some cases if we use this function
			// def.def(luabind::tostring(luabind::self));
			return def;
		}
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const BaseEntityComponent &component);
};
