// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GAMEENTITIES_H__
#define __C_GAMEENTITIES_H__
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"

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

#endif
