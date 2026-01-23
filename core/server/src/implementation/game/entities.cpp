// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.server;
import :game;

import :entities;
import :entities.components;
import :entities.registration;
import :scripting.lua;
import :server_state;
import pragma.shared;

pragma::SPlayerComponent *pragma::SGame::GetPlayer(networking::IServerClient &session) { return ServerState::Get()->GetPlayer(session); }

SBaseEntity *pragma::SGame::CreateEntity(std::string classname)
{
	if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	string::to_lower(classname);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_entity");
	pragma::util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	auto *entlua = CreateLuaEntity(classname);
	if(entlua != nullptr)
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
		Con::CWAR << "Unable to create entity '" << classname << "': Factory not found!" << Con::endl;
		return nullptr;
	}
	return factory(ServerState::Get());
}

void pragma::SGame::RemoveEntity(ecs::BaseEntity *ent)
{
	if(math::is_flag_set(ent->GetStateFlags(), ecs::BaseEntity::StateFlags::Removed))
		return;
	ent->SetStateFlag(ecs::BaseEntity::StateFlags::Removed);
	auto *s_ent = static_cast<SBaseEntity *>(ent);
	if(s_ent->IsShared()) {
		auto ID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*ent));
		if(ID != std::nullopt) {
			NetPacket p;
			networking::write_entity(p, ent);
			ServerState::Get()->SendPacket(networking::net_messages::client::ENT_REMOVE, p, networking::Protocol::SlowReliable);
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
	m_ents[idx] = nullptr;
	m_baseEnts[idx] = nullptr;
	if(idx == m_ents.size() - 1) {
		m_ents.erase(m_ents.begin() + idx);
		m_baseEnts.erase(m_baseEnts.begin() + idx);
	}
	else
		m_entIndices.push_back(idx);
	m_numEnts--;
}

void pragma::SGame::SpawnEntity(ecs::BaseEntity *ent) // Don't call directly
{
	Game::SpawnEntity(ent);
	auto ID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*ent));

	auto pMapComponent = ent->GetComponent<MapComponent>();
	if(ID != std::nullopt && (pMapComponent.valid() == false || pMapComponent->GetMapIndex() == 0)) {
		networking::ClientRecipientFilter rp {[](const networking::IServerClient &client) -> bool {
			auto *pl = client.GetPlayer();
			return pl && static_cast<SPlayerComponent *>(pl)->IsAuthed();
		}};
		SBaseEntity *sent = static_cast<SBaseEntity *>(ent);
		NetPacket p;
		p->Write<unsigned int>(*ID);
		p->Write<unsigned int>(ent->GetIndex());
		p->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
		sent->SendData(p, rp);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_CREATE, p, networking::Protocol::SlowReliable, rp);
	}
	auto hEnt = ent->GetHandle();
	CallCallbacks<void, ecs::BaseEntity *>("OnEntitySpawned", ent); // TODO: Call this after transmission for lua-entities has finished (Entity:OnPostSpawn)

	if(hEnt.valid() == false)
		return;
	auto globalNameComponent = ent->GetComponent<GlobalNameComponent>();
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

void pragma::SGame::GetEntities(std::vector<SBaseEntity *> **ents) { *ents = &m_ents; }

SBaseEntity *pragma::SGame::GetEntity(unsigned int idx)
{
	if(idx >= m_ents.size())
		return nullptr;
	return m_ents[idx];
}

void pragma::SGame::SetupEntity(ecs::BaseEntity *ent, unsigned int idx)
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
	auto pSoundEmitterComponent = sEnt->GetComponent<SSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->PrecacheSounds();
	m_ents[idx] = sEnt;
	m_baseEnts[idx] = sEnt;
	m_numEnts++;
	if(sEnt->IsPlayer())
		m_numPlayers++;
	OnEntityCreated(sEnt);
}

unsigned int pragma::SGame::GetFreeEntityIndex()
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

SBaseEntity *pragma::SGame::CreateLuaEntity(std::string classname, bool bLoadIfNotExists)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_lua_entity");
	pragma::util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	luabind::object oClass {};
	auto *ent = static_cast<SBaseEntity *>(Game::CreateLuaEntity<SLuaEntity, LuaCore::HandleHolder<SLuaEntity>>(classname, oClass, bLoadIfNotExists));
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
		return nullptr;
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

void pragma::SGame::GetPlayers(std::vector<ecs::BaseEntity *> *ents) { GetPlayers<ecs::BaseEntity>(ents); }
void pragma::SGame::GetNPCs(std::vector<ecs::BaseEntity *> *ents) { GetNPCs<ecs::BaseEntity>(ents); }
void pragma::SGame::GetWeapons(std::vector<ecs::BaseEntity *> *ents) { GetWeapons<ecs::BaseEntity>(ents); }
void pragma::SGame::GetVehicles(std::vector<ecs::BaseEntity *> *ents) { GetVehicles<ecs::BaseEntity>(ents); }

void pragma::SGame::GetPlayers(std::vector<SBaseEntity *> *ents) { GetPlayers<SBaseEntity>(ents); }
void pragma::SGame::GetNPCs(std::vector<SBaseEntity *> *ents) { GetNPCs<SBaseEntity>(ents); }
void pragma::SGame::GetWeapons(std::vector<SBaseEntity *> *ents) { GetWeapons<SBaseEntity>(ents); }
void pragma::SGame::GetVehicles(std::vector<SBaseEntity *> *ents) { GetVehicles<SBaseEntity>(ents); }

void pragma::SGame::GetPlayers(std::vector<EntityHandle> *ents)
{
	auto &players = SPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(pl->GetEntity().GetHandle());
}
void pragma::SGame::GetNPCs(std::vector<EntityHandle> *ents)
{
	auto &npcs = SAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(npc->GetEntity().GetHandle());
}
void pragma::SGame::GetWeapons(std::vector<EntityHandle> *ents)
{
	auto &weapons = SWeaponComponent::GetAll();
	ents->reserve(ents->size() + weapons.size());
	for(auto *wep : weapons)
		ents->push_back(wep->GetEntity().GetHandle());
}
void pragma::SGame::GetVehicles(std::vector<EntityHandle> *ents)
{
	auto &vehicles = SVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(vhc->GetEntity().GetHandle());
}
