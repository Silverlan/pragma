/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_UTIL_COMPONENT_HPP__
#define __LUA_UTIL_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
namespace pragma
{
	namespace lua
	{
		template<typename ...Types>
			luabind::class_<Types...> create_entity_component_class(const char *name)
		{
			auto def = luabind::class_<Types...>(name);
			def.def(luabind::tostring(luabind::self));
			return def;
		}
	};
};

#endif
