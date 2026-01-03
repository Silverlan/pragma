// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.ents;
import :client_state;
import :entities.components.listener;
import :entities.components.player;
import :entities.components.render;
import :entities.components.view_body;
import :entities.components.view_model;
import :game;

int Lua::ents::Client::get_local_player(lua::State *l)
{
	auto *pl = pragma::get_cgame()->GetLocalPlayer();
	if(pl == nullptr)
		return 0;
	pl->PushLuaObject(l);
	return 1;
}
int Lua::ents::Client::get_listener(lua::State *l)
{
	auto *listener = pragma::get_cgame()->GetListener<pragma::CListenerComponent>();
	if(listener == nullptr)
		return 0;
	listener->PushLuaObject(l);
	return 1;
}

int Lua::ents::Client::get_view_model(lua::State *l)
{
	auto *vm = pragma::get_cgame()->GetViewModel<pragma::CViewModelComponent>();
	if(vm == nullptr)
		return 0;
	vm->GetEntity().GetLuaObject().push(l);
	return 1;
}

int Lua::ents::Client::get_view_body(lua::State *l)
{
	auto *vb = pragma::get_cgame()->GetViewBody<pragma::CViewBodyComponent>();
	if(vb == nullptr)
		return 0;
	vb->GetEntity().GetLuaObject().push(l);
	return 1;
}

int Lua::ents::Client::get_instance_buffer(lua::State *l)
{
	auto &instanceBuffer = pragma::CRenderComponent::GetInstanceBuffer();
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l, instanceBuffer);
	PushInt(l, instanceBuffer->GetInstanceSize());
	return 2;
}

int Lua::ents::Client::get_instance_bone_buffer(lua::State *l)
{
	auto &instanceBuffer = pragma::get_instance_bone_buffer();
	Lua::Push<std::shared_ptr<prosper::IBuffer>>(l, instanceBuffer);
	PushInt(l, instanceBuffer->GetInstanceSize());
	return 2;
}
