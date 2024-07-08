/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_LUA_ENTITY_COMPONENTS_HPP__
#define __PRAGMA_LUA_ENTITY_COMPONENTS_HPP__

#include "pragma/lua/luaapi.h"
#include "pragma/lua/types/udm.hpp"

namespace pragma {
	class ChildComponent;
	class BaseAnimatedComponent;
	struct EntityURef;
	struct MultiEntityURef;
	struct EntityUComponentMemberRef;
};
namespace pragma::lua {
	DLLNETWORK void register_entity_component_classes(lua_State *l, luabind::module_ &mod);
	DLLNETWORK std::optional<Lua::udm_type> get_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo);
	DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, Lua::udm_type value);
	DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityURef &eref);
	DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::MultiEntityURef &eref);
	DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityUComponentMemberRef &eref);
};

#endif
