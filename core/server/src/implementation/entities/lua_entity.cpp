// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.lua;

import :entities.components;
import :game;
import :scripting.lua;
import :server_state;

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
void SLuaEntity::InitializeLuaObject(lua::State *lua) { BaseEntity::InitializeLuaObject<SLuaEntity>(lua); }

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
		pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::ENT_CREATE_LUA, p, pragma::networking::Protocol::SlowReliable, rf);
	}
}
void SLuaEntity::Remove()
{
	if(IsSpawned()) {
		// TODO: Do we need this? (If so, why?)
		NetPacket p;
		pragma::networking::write_entity(p, this);
		pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::ENT_REMOVE, p, pragma::networking::Protocol::SlowReliable);
	}
	SBaseEntity::Remove();
}
void SLuaEntity::default_Initialize(SBaseEntity *ent) {}
