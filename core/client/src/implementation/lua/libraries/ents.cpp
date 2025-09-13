// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/c_world.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/lua/classes/ldef_material.h"
#include <texturemanager/texturemanager.h>
#include "luasystem.h"
#include <buffers/prosper_uniform_resizable_buffer.hpp>

module pragma.client.scripting.lua.libraries.ents;

import pragma.client.client_state;
import pragma.client.entities.components.player;
import pragma.client.entities.components.render;

extern ClientState *client;
extern CGame *c_game;

int Lua::ents::Client::get_local_player(lua_State *l)
{
	auto *pl = c_game->GetLocalPlayer();
	if(pl == nullptr)
		return 0;
	pl->PushLuaObject(l);
	return 1;
}
int Lua::ents::Client::get_listener(lua_State *l)
{
	auto *listener = c_game->GetListener();
	if(listener == nullptr)
		return 0;
	listener->PushLuaObject(l);
	return 1;
}

int Lua::ents::Client::get_view_model(lua_State *l)
{
	auto *vm = c_game->GetViewModel();
	if(vm == nullptr)
		return 0;
	vm->GetEntity().GetLuaObject().push(l);
	return 1;
}

int Lua::ents::Client::get_view_body(lua_State *l)
{
	auto *vb = c_game->GetViewBody();
	if(vb == nullptr)
		return 0;
	vb->GetEntity().GetLuaObject().push(l);
	return 1;
}

int Lua::ents::Client::get_instance_buffer(lua_State *l)
{
	auto &instanceBuffer = pragma::CRenderComponent::GetInstanceBuffer();
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l, instanceBuffer);
	Lua::PushInt(l, instanceBuffer->GetInstanceSize());
	return 2;
}

int Lua::ents::Client::get_instance_bone_buffer(lua_State *l)
{
	auto &instanceBuffer = pragma::get_instance_bone_buffer();
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l, instanceBuffer);
	Lua::PushInt(l, instanceBuffer->GetInstanceSize());
	return 2;
}
