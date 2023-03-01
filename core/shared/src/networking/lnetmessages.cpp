/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/lua/lnetmessages.h"
#include <sharedutils/netpacket.hpp>

bool NetIncludePacketID(NetworkState *state, std::string identifier, NetPacket &src, NetPacket &tgt)
{
	if(!state->IsGameActive())
		return false;
	Game *game = state->GetGameState();
	unsigned int ID = game->GetNetMessageID(identifier);
	if(ID == 0)
		return false;
	//size_t pos = 0;
	tgt->Write<unsigned int>(ID);
	tgt->Write(src->GetData(), src->GetSize()); // TODO Avoid having to copy the packet data
	return true;
}

void Game::RegisterLuaNetMessage(std::string name, int handler) { m_luaNetMessages.insert(std::unordered_map<std::string, int>::value_type(name, handler)); }

bool Game::RegisterNetMessage(std::string name)
{
	for(int i = 1; i < m_luaNetMessageIndex.size(); i++) {
		if(m_luaNetMessageIndex[i] == name)
			return false;
	}
	m_luaNetMessageIndex.push_back(name);
	return true;
}

unsigned int Game::GetNetMessageID(std::string name)
{
	for(unsigned int i = 1; i < m_luaNetMessageIndex.size(); i++) {
		if(m_luaNetMessageIndex[i] == name)
			return i;
	}
	return 0;
}

std::string *Game::GetNetMessageIdentifier(unsigned int ID)
{
	if(ID >= m_luaNetMessageIndex.size())
		return NULL;
	return &m_luaNetMessageIndex[ID];
}
