// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/networking/iserver_client.hpp"
#include <pragma/entities/components/map_component.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>

import pragma.server.game;
import pragma.server.scripting.lua;
import pragma.server.server_state;

extern SGame *s_game;
extern ServerState *server;

SLuaEntity::SLuaEntity() : SBaseEntity {} {}
void SLuaEntity::Initialize()
{
	// We're skipping SBaseEntity on purpose, because it would overwrite our Lua class name
	BaseEntity::Initialize();
}
void SLuaEntity::SetupLua(const luabind::object &o, const std::string &className)
{
	m_className = pragma::ents::register_class_name(className);
	SetLuaObject(o);
}
bool SLuaEntity::IsScripted() const { return true; }
void SLuaEntity::InitializeLuaObject(lua_State *lua) { pragma::BaseLuaHandle::InitializeLuaObject<SLuaEntity>(lua); }

void SLuaEntity::DoSpawn()
{
	SBaseEntity::DoSpawn();
	if(IsShared()) {
		pragma::networking::ClientRecipientFilter rf {[](const pragma::networking::IServerClient &cl) -> bool { return cl.GetPlayer(); }};
		NetPacket p;
		p->WriteString(*GetClass());
		p->Write<unsigned int>(GetIndex());
		auto pMapComponent = GetComponent<pragma::MapComponent>();
		p->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
		SendData(p, rf);
		server->SendPacket("ent_create_lua", p, pragma::networking::Protocol::SlowReliable, rf);
	}
}
void SLuaEntity::Remove()
{
	if(IsSpawned()) {
		// TODO: Do we need this? (If so, why?)
		NetPacket p;
		nwm::write_entity(p, this);
		server->SendPacket("ent_remove", p, pragma::networking::Protocol::SlowReliable);
	}
	SBaseEntity::Remove();
}
void SLuaEntity::default_Initialize(SBaseEntity *ent) {}
