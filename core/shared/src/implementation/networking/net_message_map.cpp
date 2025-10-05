// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/networkdefinitions.h"

module pragma.shared;

import :networking.net_message_map;

void ClientMessageMap::PreRegisterNetMessage(std::string name, void (*)(NetPacket))
{
	if(m_netMessageIDs.find(name) != m_netMessageIDs.end())
		return;
	CLNetMessage msg;
	msg.ID = m_messageID;
	msg.handler = nullptr;
	m_netMessageIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(name, msg.ID));
	m_netMessages.insert(std::unordered_map<unsigned int, CLNetMessage>::value_type(msg.ID, msg));
	m_messageID++;
}

void ClientMessageMap::RegisterNetMessage(std::string name, void (*handler)(NetPacket))
{
	std::unordered_map<std::string, unsigned int>::iterator i = m_netMessageIDs.find(name);
	if(i != m_netMessageIDs.end()) {
		unsigned int ID = i->second;
		std::unordered_map<unsigned int, CLNetMessage>::iterator j = m_netMessages.find(ID);
		if(j != m_netMessages.end())
			j->second.handler = handler;
		return;
	}
	CLNetMessage msg;
	msg.ID = m_messageID;
	msg.handler = handler;
	m_netMessageIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(name, msg.ID));
	m_netMessages.insert(std::unordered_map<unsigned int, CLNetMessage>::value_type(msg.ID, msg));
	m_messageID++;
}

void ServerMessageMap::PreRegisterNetMessage(std::string name, void (*)(ServerClientHandle &, NetPacket))
{
	if(m_netMessageIDs.find(name) != m_netMessageIDs.end())
		return;
	SVNetMessage msg;
	msg.ID = m_messageID;
	msg.handler = nullptr;
	m_netMessageIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(name, msg.ID));
	m_netMessages.insert(std::unordered_map<unsigned int, SVNetMessage>::value_type(msg.ID, msg));
	m_messageID++;
}

void ServerMessageMap::RegisterNetMessage(std::string name, void (*handler)(ServerClientHandle &, NetPacket))
{
	std::unordered_map<std::string, unsigned int>::iterator i = m_netMessageIDs.find(name);
	if(i != m_netMessageIDs.end()) {
		unsigned int ID = i->second;
		std::unordered_map<unsigned int, SVNetMessage>::iterator j = m_netMessages.find(ID);
		if(j != m_netMessages.end())
			j->second.handler = handler;
		return;
	}
	SVNetMessage msg;
	msg.ID = m_messageID;
	msg.handler = handler;
	m_netMessageIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(name, msg.ID));
	m_netMessages.insert(std::unordered_map<unsigned int, SVNetMessage>::value_type(msg.ID, msg));
	m_messageID++;
}
