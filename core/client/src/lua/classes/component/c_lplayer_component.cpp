/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

namespace Lua
{
	namespace Player
	{
		namespace Client
		{
			static void GetViewOffset(lua_State *l,pragma::CPlayerComponent &hPl);
			static void SetViewOffset(lua_State *l,pragma::CPlayerComponent &hPl,const Vector3 &offset);
			static void IsInFirstPersonMode(lua_State *l,pragma::CPlayerComponent &hPl);
		};
	};
};
void Lua::register_cl_player_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<pragma::CPlayerComponent,pragma::BasePlayerComponent>("PlayerComponent");
	def.def("SetViewOffset",&Lua::Player::Client::SetViewOffset);
	def.def("GetViewOffset",&Lua::Player::Client::GetViewOffset);
	def.def("IsInFirstPersonMode",&Lua::Player::Client::IsInFirstPersonMode);
	module[def];
}
void Lua::Player::Client::GetViewOffset(lua_State *l,pragma::CPlayerComponent &hPl)
{
	
	Lua::Push<Vector3>(l,hPl.GetViewOffset());
}

void Lua::Player::Client::SetViewOffset(lua_State *l,pragma::CPlayerComponent &hPl,const Vector3 &offset)
{
	
	hPl.SetViewOffset(offset);
}

void Lua::Player::Client::IsInFirstPersonMode(lua_State *l,pragma::CPlayerComponent &hPl)
{
	
	Lua::PushBool(l,hPl.IsInFirstPersonMode());
}
