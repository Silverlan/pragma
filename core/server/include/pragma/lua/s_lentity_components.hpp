// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LENTITY_COMPONENTS_HPP__
#define __S_LENTITY_COMPONENTS_HPP__

#include "pragma/serverdefinitions.h"

struct lua_State;
namespace luabind {
	class module_;
};
namespace Lua {
	DLLSERVER void register_sv_ai_component(lua_State *l, luabind::module_ &module);
	DLLSERVER void register_sv_character_component(lua_State *l, luabind::module_ &module);
	DLLSERVER void register_sv_player_component(lua_State *l, luabind::module_ &module);
	DLLSERVER void register_sv_vehicle_component(lua_State *l, luabind::module_ &module);
	DLLSERVER void register_sv_weapon_component(lua_State *l, luabind::module_ &module);
};

#endif
