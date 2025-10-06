
#include "pragma/lua/ostream_operator_alias.hpp"

#include "pragma/lua/luaapi.h"

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/lua/s_lentity_components.hpp"

import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.server_state;

namespace Lua {
	namespace Player {
		namespace Server {
			static void register_class(luabind::class_<pragma::SPlayerComponent, EntityHandle> &classDef);
			static void Respawn(lua_State *l, pragma::SPlayerComponent &hEnt);
			static void SetActionInput(lua_State *l, pragma::SPlayerComponent &hPl, UInt32 input, Bool pressed);
			static bool SendResource(lua_State *l, pragma::SPlayerComponent &hPl, const std::string &name);
		};
	};
};

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BasePlayerComponent);

void Lua::register_sv_player_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::SPlayerComponent, pragma::BasePlayerComponent>("PlayerComponent");
	def.def("Respawn", &Lua::Player::Server::Respawn);
	def.def("SetActionInput", &Lua::Player::Server::SetActionInput);
	def.def("Kick", &pragma::SPlayerComponent::Kick);
	def.def("SendResource", &Lua::Player::Server::SendResource);
	module[def];
}
void Lua::Player::Server::Respawn(lua_State *l, pragma::SPlayerComponent &hEnt)
{
	auto charComponent = hEnt.GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->Respawn();
}

void Lua::Player::Server::SetActionInput(lua_State *l, pragma::SPlayerComponent &hPl, UInt32 input, Bool pressed)
{
	auto *actionInputC = hPl.GetActionInputController();
	if(actionInputC)
		actionInputC->SetActionInput(static_cast<Action>(input), pressed);
}

bool Lua::Player::Server::SendResource(lua_State *l, pragma::SPlayerComponent &hPl, const std::string &name) { return hPl.SendResource(name); }
