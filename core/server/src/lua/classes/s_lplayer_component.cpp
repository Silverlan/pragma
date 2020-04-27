/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/player.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/luaapi.h>
#include <pragma/lua/lentity_components_base_types.hpp>

namespace Lua
{
	namespace Player
	{
		namespace Server
		{
			static void register_class(luabind::class_<SPlayerHandle,EntityHandle> &classDef);
			static void Respawn(lua_State *l,SPlayerHandle &hEnt);
			static void SetActionInput(lua_State *l,SPlayerHandle &hPl,UInt32 input,Bool pressed);
			static void Kick(lua_State *l,SPlayerHandle &hPl,const std::string &reason);
			static void SendResource(lua_State *l,SPlayerHandle &hPl,const std::string &name);
		};
	};
};

extern DLLSERVER ServerState *server;

void Lua::register_sv_player_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<SPlayerHandle,BaseEntityComponentHandle>("PlayerComponent");
	Lua::register_base_player_component_methods<luabind::class_<SPlayerHandle,BaseEntityComponentHandle>,SPlayerHandle>(l,def);
	def.def("Respawn",&Lua::Player::Server::Respawn);
	def.def("SetActionInput",&Lua::Player::Server::SetActionInput);
	def.def("Kick",&Lua::Player::Server::Kick);
	def.def("SendResource",&Lua::Player::Server::SendResource);
	module[def];
}
void Lua::Player::Server::Respawn(lua_State *l,SPlayerHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto charComponent = hEnt->GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->Respawn();
}

void Lua::Player::Server::SetActionInput(lua_State *l,SPlayerHandle &hPl,UInt32 input,Bool pressed)
{
	pragma::Lua::check_component(l,hPl);
	hPl->SetActionInput(static_cast<Action>(input),pressed);
}

void Lua::Player::Server::Kick(lua_State *l,SPlayerHandle &hPl,const std::string &reason)
{
	pragma::Lua::check_component(l,hPl);
	hPl->Kick(reason);
}
void Lua::Player::Server::SendResource(lua_State *l,SPlayerHandle &hPl,const std::string &name)
{
	pragma::Lua::check_component(l,hPl);
	Lua::PushBool(l,hPl->SendResource(name));
}
