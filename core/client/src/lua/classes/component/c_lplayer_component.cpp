// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

namespace Lua {
	namespace Player {
		namespace Client {
			static void IsInFirstPersonMode(lua_State *l, pragma::CPlayerComponent &hPl);
		};
	};
};
void Lua::register_cl_player_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::CPlayerComponent, pragma::BasePlayerComponent>("PlayerComponent");
	def.def("IsInFirstPersonMode", &Lua::Player::Client::IsInFirstPersonMode);
	module[def];
}

void Lua::Player::Client::IsInFirstPersonMode(lua_State *l, pragma::CPlayerComponent &hPl) { Lua::PushBool(l, hPl.IsInFirstPersonMode()); }
