// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/entityfactories.h"
#include "luasystem.h"
#include <pragma/lua/handle_holder.hpp>
#include <sharedutils/util_string.h>
#include <pragma/debug/intel_vtune.hpp>
#include <pragma/game/game_lua_entity.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

import pragma.client.entities;
import pragma.client.entities.components;

extern ClientState *client;

template<typename TCPPM>
TCPPM *CGame::GetListener()
{
	if(m_listener.expired())
		return NULL;
	return static_cast<pragma::CListenerComponent*>(m_listener.get());
}
template pragma::CListenerComponent* CGame::GetListener<pragma::CListenerComponent>();
pragma::CPlayerComponent *CGame::GetLocalPlayer()
{
	if(m_plLocal.expired())
		return NULL;
	return m_plLocal.get();
}

CBaseEntity *CGame::CreateEntity(std::string classname)
{
	if(umath::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	StringToLower(classname);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_entity");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	CBaseEntity *entlua = CreateLuaEntity(classname);
	if(entlua != NULL)
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
		Con::cwar << "Unable to create entity '" << classname << "': Factory not found!" << Con::endl;
		return NULL;
	}
	return factory(client);
}

void CGame::RemoveEntity(BaseEntity *ent)
{
	if(umath::is_flag_set(ent->GetStateFlags(), BaseEntity::StateFlags::Removed))
		return;
	ent->SetStateFlag(BaseEntity::StateFlags::Removed);
	if(ent->IsPlayer())
		m_numPlayers--;
	unsigned int cIdx = static_cast<CBaseEntity *>(ent)->GetClientIndex();
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
		m_shEnts[idx] = NULL;
		m_shBaseEnts[idx] = NULL;
		if(idx == m_shEnts.size() - 1) {
			m_shEnts.erase(m_shEnts.begin() + idx);
			m_shBaseEnts.erase(m_shBaseEnts.begin() + idx);
		}
	}
	m_ents[cIdx] = NULL;
	m_baseEnts[cIdx] = NULL;
	if(cIdx == m_ents.size() - 1) {
		m_ents.erase(m_ents.begin() + cIdx);
		m_baseEnts.erase(m_baseEnts.begin() + cIdx);
	}
	else
		m_entIndices.push_back(cIdx);
	m_numEnts--;
}

void CGame::UpdateEntityModel(CBaseEntity *ent) { CallCallbacks<void, CBaseEntity *>("UpdateEntityModel", ent); }

void CGame::SpawnEntity(BaseEntity *ent)
{
	Game::SpawnEntity(ent);
	CallCallbacks<void, BaseEntity *>("OnEntitySpawned", ent);
}

void CGame::GetEntities(std::vector<CBaseEntity *> **ents) { *ents = &m_ents; }
void CGame::GetEntities(std::vector<BaseEntity *> **ents) { *ents = &m_baseEnts; }
void CGame::GetSharedEntities(std::vector<CBaseEntity *> **ents) { *ents = &m_shEnts; }
void CGame::GetSharedEntities(std::vector<BaseEntity *> **ents) { *ents = &m_shBaseEnts; }

CBaseEntity *CGame::GetEntity(unsigned int idx)
{
	if(idx >= m_shEnts.size())
		return NULL;
	return m_shEnts[idx];
}

BaseEntity *CGame::GetEntityByLocalIndex(uint32_t idx) { return GetEntityByClientIndex(idx); }

CBaseEntity *CGame::GetEntityByClientIndex(unsigned int idx)
{
	if(idx >= m_ents.size())
		return NULL;
	return m_ents[idx];
}

CBaseEntity *CGame::CreateLuaEntity(std::string classname, unsigned int idx, bool bLoadIfNotExists)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("create_lua_entity");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	luabind::object oClass {};
	auto *ent = static_cast<CBaseEntity *>(Game::CreateLuaEntity<CLuaEntity, pragma::lua::HandleHolder<CLuaEntity>>(classname, oClass, bLoadIfNotExists));
	if(ent == nullptr)
		return nullptr;
	SetupEntity(ent, idx);

	auto *info = m_luaEnts->GetEntityInfo(classname);
	assert(info);
	for(auto componentId : info->components)
		ent->AddComponent(componentId);

	return ent;
}

CBaseEntity *CGame::CreateLuaEntity(std::string classname, bool bLoadIfNotExists) { return CreateLuaEntity(classname, GetFreeEntityIndex(), bLoadIfNotExists); }

void CGame::SetupEntity(BaseEntity *ent, unsigned int idx)
{
	if(idx < m_shEnts.size()) {
		if(m_shEnts[idx] != nullptr) {
			Con::cwar << "New entity " << ent << " shares server index " << idx << " with existing entity " << *m_shEnts[idx] << "! Removing existing..." << Con::endl;
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
	auto *cEnt = static_cast<CBaseEntity *>(ent);
	auto *scene = GetScene<pragma::CSceneComponent>();
	if(scene)
		cEnt->AddToScene(*scene); // Add to default scene automatically
	cEnt->Construct(idx, clIdx);
	cEnt->PrecacheModels();
	auto pSoundEmitterComponent = cEnt->GetComponent<pragma::CSoundEmitterComponent>();
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
unsigned int CGame::GetFreeEntityIndex() { return 0; }

template<class T>
void CGame::GetPlayers(std::vector<T *> *ents)
{
	auto &players = pragma::CPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(&pl->GetEntity());
}

template<class T>
void CGame::GetNPCs(std::vector<T *> *ents)
{
	auto &npcs = pragma::CAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(&npc->GetEntity());
}

template<class T>
void CGame::GetWeapons(std::vector<T *> *ents)
{
	auto &weapons = pragma::CWeaponComponent::GetAll();
	ents->reserve(ents->size() + weapons.size());
	for(auto *wp : weapons)
		ents->push_back(&wp->GetEntity());
}

template<class T>
void CGame::GetVehicles(std::vector<T *> *ents)
{
	auto &vehicles = pragma::CVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(&vhc->GetEntity());
}

void CGame::GetPlayers(std::vector<BaseEntity *> *ents) { GetPlayers<BaseEntity>(ents); }
void CGame::GetNPCs(std::vector<BaseEntity *> *ents) { GetNPCs<BaseEntity>(ents); }
void CGame::GetWeapons(std::vector<BaseEntity *> *ents) { GetWeapons<BaseEntity>(ents); }
void CGame::GetVehicles(std::vector<BaseEntity *> *ents) { GetVehicles<BaseEntity>(ents); }

void CGame::GetPlayers(std::vector<EntityHandle> *ents)
{
	auto &players = pragma::CPlayerComponent::GetAll();
	ents->reserve(ents->size() + players.size());
	for(auto *pl : players)
		ents->push_back(pl->GetEntity().GetHandle());
}
void CGame::GetNPCs(std::vector<EntityHandle> *ents)
{
	auto &npcs = pragma::CAIComponent::GetAll();
	ents->reserve(ents->size() + npcs.size());
	for(auto *npc : npcs)
		ents->push_back(npc->GetEntity().GetHandle());
}
void CGame::GetWeapons(std::vector<EntityHandle> *ents)
{
	/*std::vector<CBaseWeapon*> *weps;
	CBaseWeapon::GetAll(&weps);
	for(unsigned int i=0;i<weps->size();i++)
		ents->push_back((*weps)[i]->GetHandle());*/
}
void CGame::GetVehicles(std::vector<EntityHandle> *ents)
{
	auto &vehicles = pragma::CVehicleComponent::GetAll();
	ents->reserve(ents->size() + vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(vhc->GetEntity().GetHandle());
}
