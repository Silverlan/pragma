/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/entityfactories.h"
#include "pragma/lua/classes/c_lua_entity.h"
#include "luasystem.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/game/c_game_entities.h"
#include <sharedutils/util_string.h>
#include <pragma/game/game_lua_entity.hpp>

extern EntityClassMap<CBaseEntity> *g_ClientEntityFactories;
pragma::CListenerComponent *CGame::GetListener()
{
	if(m_listener.expired())
		return NULL;
	return m_listener.get();
}
pragma::CPlayerComponent *CGame::GetLocalPlayer()
{
	if(m_plLocal.expired())
		return NULL;
	return m_plLocal.get();
}

CBaseEntity *CGame::CreateEntity(std::string classname)
{
	StringToLower(classname);
	CBaseEntity *entlua = CreateLuaEntity(classname);
	if(entlua != NULL)
		return entlua;
	CBaseEntity*(*factory)(void) = g_ClientEntityFactories->FindFactory(classname);
	if(factory == NULL)
	{
		static auto skipSecondAttempt = false;
		if(skipSecondAttempt == false && LoadLuaEntityByClass(classname) == true)
		{
			skipSecondAttempt = true;
			auto *r = CreateEntity(classname);
			skipSecondAttempt = false;
			return r;
		}
		Con::cwar<<"WARNING: Unable to create entity '"<<classname<<"': Factory not found!"<<Con::endl;
		return NULL;
	}
	return factory();
}

void CGame::RemoveEntity(BaseEntity *ent)
{
	if(ent->IsPlayer())
		m_numPlayers--;
	unsigned int cIdx = static_cast<CBaseEntity*>(ent)->GetClientIndex();
	unsigned int idx = ent->GetIndex();
	m_ents[cIdx]->OnRemove();
	delete m_ents[cIdx];
	if(idx > 0)
	{
		m_shEnts[idx] = NULL;
		m_shBaseEnts[idx] = NULL;
		if(idx == m_shEnts.size() -1)
		{
			m_shEnts.erase(m_shEnts.begin() +idx);
			m_shBaseEnts.erase(m_shBaseEnts.begin() +idx);
		}
	}
	m_ents[cIdx] = NULL;
	m_baseEnts[cIdx] = NULL;
	if(cIdx == m_ents.size() -1)
	{
		m_ents.erase(m_ents.begin() +cIdx);
		m_baseEnts.erase(m_baseEnts.begin() +cIdx);
	}
	else
		m_entIndices.push_back(cIdx);
	m_numEnts--;
}

void CGame::UpdateEntityModel(CBaseEntity *ent)
{
	CallCallbacks<void,CBaseEntity*>("UpdateEntityModel",ent);
}

void CGame::SpawnEntity(BaseEntity *ent)
{
	Game::SpawnEntity(ent);
	CallCallbacks<void,BaseEntity*>("OnEntitySpawned",ent);
}

void CGame::GetEntities(std::vector<CBaseEntity*> **ents) {*ents = &m_ents;}
void CGame::GetEntities(std::vector<BaseEntity*> **ents) {*ents = &m_baseEnts;}
void CGame::GetSharedEntities(std::vector<CBaseEntity*> **ents) {*ents = &m_shEnts;}
void CGame::GetSharedEntities(std::vector<BaseEntity*> **ents) {*ents = &m_shBaseEnts;}

CBaseEntity *CGame::GetEntity(unsigned int idx)
{
	if(idx >= m_shEnts.size())
		return NULL;
	return m_shEnts[idx];
}

BaseEntity *CGame::GetEntityByLocalIndex(uint32_t idx) {return GetEntityByClientIndex(idx);}

CBaseEntity *CGame::GetEntityByClientIndex(unsigned int idx)
{
	if(idx >= m_ents.size())
		return NULL;
	return m_ents[idx];
}

CBaseEntity *CGame::CreateLuaEntity(std::string classname,unsigned int idx,bool bLoadIfNotExists)
{
	luabind::object oClass {};
	auto *ent = static_cast<CBaseEntity*>(Game::CreateLuaEntity<CLuaEntity,CLuaEntityHandle>(classname,oClass,bLoadIfNotExists));
	if(ent == nullptr)
		return nullptr;
	SetupEntity(ent,idx);
	return ent;
}

CBaseEntity *CGame::CreateLuaEntity(std::string classname,bool bLoadIfNotExists)
{
	return CreateLuaEntity(classname,GetFreeEntityIndex(),bLoadIfNotExists);
}

void CGame::SetupEntity(BaseEntity *ent,unsigned int idx)
{
	if(idx < m_shEnts.size())
	{
		if(m_shEnts[idx] != nullptr)
		{
			Con::cwar<<"WARNING: New entity ";
			ent->print(Con::cout);
			Con::cwar<<" shares server index "<<idx<<" with existing entity ";
			m_shEnts[idx]->print(Con::cout);
			Con::cwar<<"! Removing existing..."<<Con::endl;
			auto *entOther = GetEntity(idx);
			RemoveEntity(entOther);
		}
	}
	else
	{
		for(auto i=m_shEnts.size();i<idx;i++)
		{
			m_shEnts.push_back(nullptr);
			m_shBaseEnts.push_back(nullptr);
		}
		m_shEnts.push_back(nullptr);
		m_shBaseEnts.push_back(nullptr);
	}

	unsigned int clIdx;
	if(!m_entIndices.empty())
	{
		clIdx = m_entIndices[0];
		m_entIndices.pop_front();
	}
	else
	{
		m_ents.push_back(nullptr);
		m_baseEnts.push_back(nullptr);
		clIdx = CUInt32(m_ents.size()) -1;
	}
	auto *cEnt = static_cast<CBaseEntity*>(ent);
	auto *scene = GetScene();
	if(scene)
		cEnt->AddToScene(*scene); // Add to default scene automatically
	cEnt->Construct(idx,clIdx);
	cEnt->PrecacheModels();
	auto pSoundEmitterComponent = cEnt->GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->PrecacheSounds();
	m_ents[clIdx] = cEnt;
	m_baseEnts[clIdx] = cEnt;
	if(idx > 0)
	{
		m_shEnts[idx] = cEnt;
		m_shBaseEnts[idx] = cEnt;
	}
	m_numEnts++;
	if(cEnt->IsPlayer()) m_numPlayers++;

	OnEntityCreated(cEnt);
}
unsigned int CGame::GetFreeEntityIndex() {return 0;}

void CGame::GetPlayers(std::vector<BaseEntity*> *ents) {GetPlayers<BaseEntity>(ents);}
void CGame::GetNPCs(std::vector<BaseEntity*> *ents) {GetNPCs<BaseEntity>(ents);}
void CGame::GetWeapons(std::vector<BaseEntity*> *ents) {GetWeapons<BaseEntity>(ents);}
void CGame::GetVehicles(std::vector<BaseEntity*> *ents) {GetVehicles<BaseEntity>(ents);}

void CGame::GetPlayers(std::vector<EntityHandle> *ents)
{
	auto &players = pragma::CPlayerComponent::GetAll();
	ents->reserve(ents->size() +players.size());
	for(auto *pl : players)
		ents->push_back(pl->GetEntity().GetHandle());
}
void CGame::GetNPCs(std::vector<EntityHandle> *ents)
{
	auto &npcs = pragma::CAIComponent::GetAll();
	ents->reserve(ents->size() +npcs.size());
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
	ents->reserve(ents->size() +vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(vhc->GetEntity().GetHandle());
}
