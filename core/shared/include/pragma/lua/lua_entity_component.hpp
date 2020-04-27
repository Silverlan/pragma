/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LUA_ENTITY_COMPONENT_HPP__
#define __LUA_ENTITY_COMPONENT_HPP__

namespace pragma
{
	namespace Lua
	{
		template<class TComponentHandle>
			bool check_component(lua_State *l,const TComponentHandle &component)
		{
			if(component.expired())
			{
				lua_pushstring(l,"Attempted to use a NULL component");
				lua_error(l);
				return false;
			}
			return true;
		}
	};
};

#endif
