#ifndef __S_GAMEENTITIES_H__
#define __S_GAMEENTITIES_H__
#include "pragma/game/s_game.h"
#include "pragma/entities/player.h"
#include "pragma/entities/components/s_player_component.hpp"

template<class T>
	void SGame::GetPlayers(std::vector<T*> *ents)
{
	auto &players = pragma::SPlayerComponent::GetAll();
	ents->reserve(ents->size() +players.size());
	for(auto *pl : players)
		ents->push_back(static_cast<T*>(&pl->GetEntity()));
}

template<class T>
	void SGame::GetNPCs(std::vector<T*> *ents)
{
	auto &npcs = pragma::SPlayerComponent::GetAll();
	ents->reserve(ents->size() +npcs.size());
	for(auto *npc : npcs)
		ents->push_back(static_cast<T*>(&npc->GetEntity()));
}

template<class T>
	void SGame::GetWeapons(std::vector<T*> *ents)
{
	auto &weapons = pragma::SPlayerComponent::GetAll();
	ents->reserve(ents->size() +weapons.size());
	for(auto *wep : weapons)
		ents->push_back(static_cast<T*>(&wep->GetEntity()));
}

template<class T>
	void SGame::GetVehicles(std::vector<T*> *ents)
{
	auto &vehicles = pragma::SPlayerComponent::GetAll();
	ents->reserve(ents->size() +vehicles.size());
	for(auto *vhc : vehicles)
		ents->push_back(static_cast<T*>(&vhc->GetEntity()));
}

#endif