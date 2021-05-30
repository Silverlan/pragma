/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LENTITY_COMPONENTS_HPP__
#define __S_LENTITY_COMPONENTS_HPP__

#include "pragma/serverdefinitions.h"

struct lua_State;
namespace luabind {class module_;};
namespace Lua
{
	DLLSERVER void register_sv_ai_component(lua_State *l,luabind::module_ &module);
	DLLSERVER void register_sv_character_component(lua_State *l,luabind::module_ &module);
	DLLSERVER void register_sv_player_component(lua_State *l,luabind::module_ &module);
	DLLSERVER void register_sv_vehicle_component(lua_State *l,luabind::module_ &module);
	DLLSERVER void register_sv_weapon_component(lua_State *l,luabind::module_ &module);
};

#endif
