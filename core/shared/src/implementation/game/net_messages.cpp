// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

bool NetIncludePacketID(pragma::NetworkState *state, std::string identifier, NetPacket &src, NetPacket &tgt)
{
	if(!state->IsGameActive())
		return false;
	pragma::Game *game = state->GetGameState();
	unsigned int ID = game->GetNetMessageID(identifier);
	if(ID == 0)
		return false;
	//size_t pos = 0;
	tgt->Write<unsigned int>(ID);
	tgt->Write(src->GetData(), src->GetSize()); // TODO Avoid having to copy the packet data
	return true;
}

void pragma::Game::RegisterLuaNetMessage(std::string name, int handler) { m_luaNetMessages.insert(std::unordered_map<std::string, int>::value_type(name, handler)); }

bool pragma::Game::RegisterNetMessage(std::string name)
{
	for(int i = 1; i < m_luaNetMessageIndex.size(); i++) {
		if(m_luaNetMessageIndex[i] == name)
			return false;
	}
	m_luaNetMessageIndex.push_back(name);
	return true;
}

unsigned int pragma::Game::GetNetMessageID(std::string name)
{
	for(unsigned int i = 1; i < m_luaNetMessageIndex.size(); i++) {
		if(m_luaNetMessageIndex[i] == name)
			return i;
	}
	return 0;
}

std::string *pragma::Game::GetNetMessageIdentifier(unsigned int ID)
{
	if(ID >= m_luaNetMessageIndex.size())
		return nullptr;
	return &m_luaNetMessageIndex[ID];
}
