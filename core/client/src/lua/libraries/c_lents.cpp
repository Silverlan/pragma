/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lents.h"
#include "pragma/entities/c_world.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/lua/classes/ldef_material.h"
#include <texturemanager/texturemanager.h>
#include "luasystem.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <buffers/prosper_uniform_resizable_buffer.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

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
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l,instanceBuffer);
	Lua::PushInt(l,instanceBuffer->GetInstanceSize());
	return 2;
}

int Lua::ents::Client::get_instance_bone_buffer(lua_State *l)
{
	auto &instanceBuffer = pragma::get_instance_bone_buffer();
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l,instanceBuffer);
	Lua::PushInt(l,instanceBuffer->GetInstanceSize());
	return 2;
}
