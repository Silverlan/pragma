// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :game;
import :client_state;
import :entities;
import :entities.components;
import :game;

template<typename TCPPM>
TCPPM *pragma::CGame::GetListener()
{
	if(m_listener.expired())
		return nullptr;
	return static_cast<CListenerComponent *>(m_listener.get());
}
template pragma::CListenerComponent *pragma::CGame::GetListener<pragma::CListenerComponent>();
pragma::CPlayerComponent *pragma::CGame::GetLocalPlayer()
{
	if(m_plLocal.expired())
		return nullptr;
	return m_plLocal.get();
}

pragma::ecs::CBaseEntity *pragma::CGame::CreateEntity(std::string classname)
{
	if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	string::to_lower(classname);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_entity");
	pragma::util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	ecs::CBaseEntity *entlua = CreateLuaEntity(classname);
	if(entlua != nullptr)
		return entlua;
	auto factory = client_entities::ClientEntityRegistry::Instance().FindFactory(classname);
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
	return factory(get_client_state());
}

void pragma::CGame::RemoveEntity(ecs::BaseEntity *ent)
{
	if(math::is_flag_set(ent->GetStateFlags(), ecs::BaseEntity::StateFlags::Removed))
		return;
	ent->SetStateFlag(ecs::BaseEntity::StateFlags::Removed);
	if(ent->IsPlayer())
		m_numPlayers--;
	unsigned int cIdx = static_cast<ecs::CBaseEntity *>(ent)->GetClientIndex();
	unsigned int idx = ent->GetIndex();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("remove_entity");
#endif
	m_ents[cIdx]->OnRemove();
	delete m_ents[cIdx];
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
	if(idx > 0) {
		m_shEnts[idx] = nullptr;
		m_shBaseEnts[idx] = nullptr;
		if(idx == m_shEnts.size() - 1) {
			m_shEnts.erase(m_shEnts.begin() + idx);
			m_shBaseEnts.erase(m_shBaseEnts.begin() + idx);
		}
	}
	m_ents[cIdx] = nullptr;
	m_baseEnts[cIdx] = nullptr;
	if(cIdx == m_ents.size() - 1) {
		m_ents.erase(m_ents.begin() + cIdx);
		m_baseEnts.erase(m_baseEnts.begin() + cIdx);
	}
	else
		m_entIndices.push_back(cIdx);
	m_numEnts--;
}

void pragma::CGame::UpdateEntityModel(ecs::CBaseEntity *ent) { CallCallbacks<void, ecs::CBaseEntity *>("UpdateEntityModel", ent); }

void pragma::CGame::SpawnEntity(ecs::BaseEntity *ent)
{
	Game::SpawnEntity(ent);
	CallCallbacks<void, ecs::BaseEntity *>("OnEntitySpawned", ent);
}

void pragma::CGame::GetEntities(std::vector<ecs::CBaseEntity *> **ents) { *ents = &m_ents; }
void pragma::CGame::GetEntities(std::vector<ecs::BaseEntity *> **ents) { *ents = &m_baseEnts; }
void pragma::CGame::GetSharedEntities(std::vector<ecs::CBaseEntity *> **ents) { *ents = &m_shEnts; }
void pragma::CGame::GetSharedEntities(std::vector<ecs::BaseEntity *> **ents) { *ents = &m_shBaseEnts; }

pragma::ecs::CBaseEntity *pragma::CGame::GetEntity(unsigned int idx)
{
	if(idx >= m_shEnts.size())
		return nullptr;
	return m_shEnts[idx];
}

pragma::ecs::CBaseEntity *pragma::CGame::GetEntityByLocalIndex(uint32_t idx) { return GetEntityByClientIndex(idx); }

pragma::ecs::CBaseEntity *pragma::CGame::GetEntityByClientIndex(unsigned int idx)
{
	if(idx >= m_ents.size())
		return nullptr;
	return m_ents[idx];
}

pragma::ecs::CBaseEntity *pragma::CGame::CreateLuaEntity(std::string classname, unsigned int idx, bool bLoadIfNotExists)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_lua_entity");
	pragma::util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	luabind::object oClass {};
	auto *ent = static_cast<ecs::CBaseEntity *>(Game::CreateLuaEntity<CLuaEntity, LuaCore::HandleHolder<CLuaEntity>>(classname, oClass, bLoadIfNotExists));
	if(ent == nullptr)
		return nullptr;
	SetupEntity(ent, idx);

	auto *info = m_luaEnts->GetEntityInfo(classname);
	assert(info);
	for(auto componentId : info->components)
		ent->AddComponent(componentId);

	return ent;
}

pragma::ecs::CBaseEntity *pragma::CGame::CreateLuaEntity(std::string classname, bool bLoadIfNotExists) { return CreateLuaEntity(classname, GetFreeEntityIndex(), bLoadIfNotExists); }

void pragma::CGame::SetupEntity(ecs::BaseEntity *ent, unsigned int idx)
{
	if(idx < m_shEnts.size()) {
		if(m_shEnts[idx] != nullptr) {
			Con::CWAR << "New entity " << ent << " shares server index " << idx << " with existing entity " << *m_shEnts[idx] << "! Removing existing..." << Con::endl;
			auto *entOther = GetEntity(idx);
			RemoveEntity(entOther);
		}
	}
	else {
		for(auto i = m_shEnts.size(); i < idx; i++) {
			m_shEnts.push_back(nullptr);
			m_shBaseEnts.push_back(nullptr);
		}
		m_shEnts.push_back(nullptr);
		m_shBaseEnts.push_back(nullptr);
	}

	unsigned int clIdx;
	if(!m_entIndices.empty()) {
		clIdx = m_entIndices[0];
		m_entIndices.pop_front();
	}
	else {
		m_ents.push_back(nullptr);
		m_baseEnts.push_back(nullptr);
		clIdx = CUInt32(m_ents.size()) - 1;
	}
	auto *cEnt = static_cast<ecs::CBaseEntity *>(ent);
	auto *scene = GetScene<CSceneComponent>();
	if(scene)
		cEnt->AddToScene(*scene); // Add to default scene automatically
	cEnt->Construct(idx, clIdx);
	cEnt->PrecacheModels();
	auto pSoundEmitterComponent = cEnt->GetComponent<CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->PrecacheSounds();
	m_ents[clIdx] = cEnt;
	m_baseEnts[clIdx] = cEnt;
	if(idx > 0) {
		m_shEnts[idx] = cEnt;
		m_shBaseEnts[idx] = cEnt;
	}
	m_numEnts++;
	if(cEnt->IsPlayer())
		m_numPlayers++;

	OnEntityCreated(cEnt);
}
unsigned int pragma::CGame::GetFreeEntityIndex() { return 0; }

template<class T>
void pragma::CGame::GetPlayers(std::vector<T *> *ents)
{
	auto &players = CPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(&pl->GetEntity());
}

template<class T>
void pragma::CGame::GetNPCs(std::vector<T *> *ents)
{
	auto &npcs = CAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(&npc->GetEntity());
}

template<class T>
void pragma::CGame::GetWeapons(std::vector<T *> *ents)
{
	auto &weapons = CWeaponComponent::GetAll();
	ents->reserve(ents->size() + weapons.size());
	for(auto *wp : weapons)
		ents->push_back(&wp->GetEntity());
}

template<class T>
void pragma::CGame::GetVehicles(std::vector<T *> *ents)
{
	auto &vehicles = CVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(&vhc->GetEntity());
}

void pragma::CGame::GetPlayers(std::vector<ecs::BaseEntity *> *ents) { GetPlayers<ecs::BaseEntity>(ents); }
void pragma::CGame::GetNPCs(std::vector<ecs::BaseEntity *> *ents) { GetNPCs<ecs::BaseEntity>(ents); }
void pragma::CGame::GetWeapons(std::vector<ecs::BaseEntity *> *ents) { GetWeapons<ecs::BaseEntity>(ents); }
void pragma::CGame::GetVehicles(std::vector<ecs::BaseEntity *> *ents) { GetVehicles<ecs::BaseEntity>(ents); }

void pragma::CGame::GetPlayers(std::vector<EntityHandle> *ents)
{
	auto &players = CPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(pl->GetEntity().GetHandle());
}
void pragma::CGame::GetNPCs(std::vector<EntityHandle> *ents)
{
	auto &npcs = CAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(npc->GetEntity().GetHandle());
}
void pragma::CGame::GetWeapons(std::vector<EntityHandle> *ents)
{
	/*std::vector<CBaseWeapon*> *weps;
	CBaseWeapon::GetAll(&weps);
	for(unsigned int i=0;i<weps->size();i++)
		ents->push_back((*weps)[i]->GetHandle());*/
}
void pragma::CGame::GetVehicles(std::vector<EntityHandle> *ents)
{
	auto &vehicles = CVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(vhc->GetEntity().GetHandle());
}
