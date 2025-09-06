// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include <servermanager/sv_nwm_recipientfilter.h>
#include "luasystem.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/debug/intel_vtune.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/lua/lua_entity_type.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/game/game_lua_entity.hpp>
#include <udm.hpp>

import pragma.entities.components;
import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.entities.registration;
import pragma.server.game;
import pragma.server.scripting.lua;
import pragma.server.server_state;

extern ServerState *server;
pragma::SPlayerComponent *SGame::GetPlayer(pragma::networking::IServerClient &session) { return server->GetPlayer(session); }

SBaseEntity *SGame::CreateEntity(std::string classname)
{
	if(umath::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	StringToLower(classname);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_entity");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	auto *entlua = CreateLuaEntity(classname);
	if(entlua != NULL)
		return entlua;
	auto factory = server_entities::ServerEntityRegistry::Instance().FindFactory(classname);
	if(!factory) {
		static std::unordered_set<std::string> skipSet;
		if(skipSet.find(classname) == skipSet.end() && LoadLuaEntityByClass(classname) == true) {
			skipSet.insert(classname);
			auto *r = CreateEntity(classname);
			skipSet.erase(classname);
			return r;
		}
		Con::cwar << "Unable to create entity '" << classname << "': Factory not found!" << Con::endl;
		return NULL;
	}
	return factory(server);
}

void SGame::RemoveEntity(BaseEntity *ent)
{
	if(umath::is_flag_set(ent->GetStateFlags(), BaseEntity::StateFlags::Removed))
		return;
	ent->SetStateFlag(BaseEntity::StateFlags::Removed);
	auto *s_ent = static_cast<SBaseEntity *>(ent);
	if(s_ent->IsShared()) {
		auto ID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*ent));
		if(ID != std::nullopt) {
			NetPacket p;
			nwm::write_entity(p, ent);
			server->SendPacket("ent_remove", p, pragma::networking::Protocol::SlowReliable);
		}
	}
	if(ent->IsPlayer())
		m_numPlayers--;
	unsigned int idx = ent->GetIndex();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("remove_entity");
#endif
	m_ents[idx]->OnRemove();
	delete m_ents[idx];
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
	m_ents[idx] = NULL;
	m_baseEnts[idx] = NULL;
	if(idx == m_ents.size() - 1) {
		m_ents.erase(m_ents.begin() + idx);
		m_baseEnts.erase(m_baseEnts.begin() + idx);
	}
	else
		m_entIndices.push_back(idx);
	m_numEnts--;
}

void SGame::SpawnEntity(BaseEntity *ent) // Don't call directly
{
	Game::SpawnEntity(ent);
	auto ID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*ent));

	auto pMapComponent = ent->GetComponent<pragma::MapComponent>();
	if(ID != std::nullopt && (pMapComponent.valid() == false || pMapComponent->GetMapIndex() == 0)) {
		pragma::networking::ClientRecipientFilter rp {[](const pragma::networking::IServerClient &client) -> bool {
			auto *pl = client.GetPlayer();
			return pl && pl->IsAuthed();
		}};
		SBaseEntity *sent = static_cast<SBaseEntity *>(ent);
		NetPacket p;
		p->Write<unsigned int>(*ID);
		p->Write<unsigned int>(ent->GetIndex());
		p->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
		sent->SendData(p, rp);
		server->SendPacket("ent_create", p, pragma::networking::Protocol::SlowReliable, rp);
	}
	auto hEnt = ent->GetHandle();
	CallCallbacks<void, BaseEntity *>("OnEntitySpawned", ent); // TODO: Call this after transmission for lua-entities has finished (Entity:OnPostSpawn)

	if(hEnt.valid() == false)
		return;
	auto globalNameComponent = ent->GetComponent<pragma::GlobalNameComponent>();
	if(globalNameComponent.valid()) {
		auto &globalName = globalNameComponent->GetGlobalName();
		auto it = m_preTransitionWorldState.find(globalName);
		if(it != m_preTransitionWorldState.end()) {
			udm::LinkedPropertyWrapper udm {*it->second};
			ent->Load(udm);
			if(hEnt.valid()) {
				// Move global entities by landmark offset between this level and the previous one.
				// This will not affect map-entities, as the delta offset will be 0 at this point,
				// however this will affect entities that are created after map-spawn, such as players.
				ent->SetPosition(ent->GetPosition() + m_deltaTransitionLandmarkOffset);

				it = m_preTransitionWorldState.find(globalName); // Iterator may have been become invalid by above function calls; Retrieve it again to be sure
				if(it != m_preTransitionWorldState.end())
					m_preTransitionWorldState.erase(it);
			}
		}
	}
}

void SGame::GetEntities(std::vector<SBaseEntity *> **ents) { *ents = &m_ents; }

SBaseEntity *SGame::GetEntity(unsigned int idx)
{
	if(idx >= m_ents.size())
		return nullptr;
	return m_ents[idx];
}

void SGame::SetupEntity(BaseEntity *ent, unsigned int idx)
{
	if(idx < m_ents.size()) {
		if(m_ents[idx] != nullptr) {
			m_ents[idx]->OnRemove();
			delete m_ents[idx];
		}
	}
	else {
		for(auto i = m_ents.size(); i < idx; i++) {
			m_ents.push_back(nullptr);
			m_baseEnts.push_back(nullptr);
			if(i > 0)
				m_entIndices.push_back(CUInt32(i));
		}
		m_ents.push_back(nullptr);
		m_baseEnts.push_back(nullptr);
	}
	auto *sEnt = static_cast<SBaseEntity *>(ent);
	sEnt->Construct(idx);
	sEnt->PrecacheModels();
	auto pSoundEmitterComponent = sEnt->GetComponent<pragma::SSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->PrecacheSounds();
	m_ents[idx] = sEnt;
	m_baseEnts[idx] = sEnt;
	m_numEnts++;
	if(sEnt->IsPlayer())
		m_numPlayers++;
	OnEntityCreated(sEnt);
}

unsigned int SGame::GetFreeEntityIndex()
{
	unsigned int idx;
	if(!m_entIndices.empty()) {
		idx = m_entIndices[0];
		m_entIndices.pop_front();
	}
	else
		idx = CUInt32(m_ents.size()) + 1;
	return idx;
}

SBaseEntity *SGame::CreateLuaEntity(std::string classname, bool bLoadIfNotExists)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_lua_entity");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	luabind::object oClass {};
	auto *ent = static_cast<SBaseEntity *>(Game::CreateLuaEntity<SLuaEntity, pragma::lua::SLuaEntityHolder>(classname, oClass, bLoadIfNotExists));
	if(ent == nullptr)
		return nullptr;
	auto oType = oClass["Type"];
	if(oType && static_cast<LuaEntityType>(luabind::object_cast_nothrow<int>(oType, 0)) == LuaEntityType::Shared)
		ent->SetShared(true);
	else
		ent->SetShared(false);
	SetupEntity(ent, GetFreeEntityIndex());

	auto *info = m_luaEnts->GetEntityInfo(classname);
	assert(info);
	for(auto componentId : info->components)
		ent->AddComponent(componentId);

	return ent;
	/*int ref = Lua::PushTable(m_lua,m_luaRefEntityTable);
	Lua::PushString(m_lua,classname);
	Lua::GetTableValue(m_lua,ref);
	if(!Lua::IsTable(m_lua,-1))
	{
		Lua::Pop(m_lua,2);
		return NULL;
	}
	int refType = Lua::PushTable(m_lua,m_luaRefEntityTypes);
	Lua::PushString(m_lua,classname);
	Lua::GetTableValue(m_lua,refType);
	int type = LUA_ENTITY_TYPE_ENTITY;
	if(Lua::IsNumber(m_lua,-1))
		type = Lua::ToInt<int>(m_lua,-1);
	Lua::Pop(m_lua,2);
	SBaseEntity *ent;
	switch(type)
	{
	case LUA_ENTITY_TYPE_WEAPON:
		ent = CreateEntity<SLuaWeapon>();
		break;
	case LUA_ENTITY_TYPE_NPC:
		ent = CreateEntity<SLuaNPC>();
		break;
	case LUA_ENTITY_TYPE_VEHICLE:
		ent = CreateEntity<SLuaVehicle>();
		break;
	default:
		ent = CreateEntity<SLuaEntity>();
	}
	int data = Lua::GetStackTop(m_lua);
	luabind::object *obj = ent->GetLuaObject();
	obj->push(m_lua);
	int idxObj = Lua::GetStackTop(m_lua);
	IncludeLuaEntityBaseClasses(m_lua,ref,idxObj,data);
	Lua::PushNil(m_lua);
	while(Lua::GetNextPair(m_lua,data) != 0)
	{
		Lua::PushValue(m_lua,-2); // We need the key for the next iteration
		Lua::Insert(m_lua,-2);
		Lua::SetTableValue(m_lua,idxObj);
	}
	dynamic_cast<BaseLuaEntity*>(ent)->SetUp(classname);
	return ent;*/
}

void SGame::GetPlayers(std::vector<BaseEntity *> *ents) { GetPlayers<BaseEntity>(ents); }
void SGame::GetNPCs(std::vector<BaseEntity *> *ents) { GetNPCs<BaseEntity>(ents); }
void SGame::GetWeapons(std::vector<BaseEntity *> *ents) { GetWeapons<BaseEntity>(ents); }
void SGame::GetVehicles(std::vector<BaseEntity *> *ents) { GetVehicles<BaseEntity>(ents); }

void SGame::GetPlayers(std::vector<SBaseEntity *> *ents) { GetPlayers<SBaseEntity>(ents); }
void SGame::GetNPCs(std::vector<SBaseEntity *> *ents) { GetNPCs<SBaseEntity>(ents); }
void SGame::GetWeapons(std::vector<SBaseEntity *> *ents) { GetWeapons<SBaseEntity>(ents); }
void SGame::GetVehicles(std::vector<SBaseEntity *> *ents) { GetVehicles<SBaseEntity>(ents); }

void SGame::GetPlayers(std::vector<EntityHandle> *ents)
{
	auto &players = pragma::SPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(pl->GetEntity().GetHandle());
}
void SGame::GetNPCs(std::vector<EntityHandle> *ents)
{
	auto &npcs = pragma::SAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(npc->GetEntity().GetHandle());
}
void SGame::GetWeapons(std::vector<EntityHandle> *ents)
{
	auto &weapons = pragma::SWeaponComponent::GetAll();
	ents->reserve(ents->size() + weapons.size());
	for(auto *wep : weapons)
		ents->push_back(wep->GetEntity().GetHandle());
}
void SGame::GetVehicles(std::vector<EntityHandle> *ents)
{
	auto &vehicles = pragma::SVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(vhc->GetEntity().GetHandle());
}
