// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/networkdefinitions.h"

module pragma.shared;

import :networking.net_message_map;

#define net_createmap(suffix, glname)                                                                                                                                                                                                                                                            \
	if(g_NetMessages##suffix == nullptr)                                                                                                                                                                                                                                                            \
		g_NetMessages##suffix = new glname##MessageMap;

#define net_newglobal(suffix, glname, args)                                                                                                                                                                                                                                                      \
	DLLNETWORK glname##MessageMap *g_NetMessages##suffix = nullptr;                                                                                                                                                                                                                                 \
	DLLNETWORK void Register##glname##NetMessage(std::string name, void (*handler)(args))                                                                                                                                                                                                        \
	{                                                                                                                                                                                                                                                                                            \
		net_createmap(suffix, glname);                                                                                                                                                                                                                                                           \
		if(handler == nullptr)                                                                                                                                                                                                                                                                      \
			g_NetMessages##suffix->PreRegisterNetMessage(name, handler);                                                                                                                                                                                                                         \
		else                                                                                                                                                                                                                                                                                     \
			g_NetMessages##suffix->RegisterNetMessage(name, handler);                                                                                                                                                                                                                            \
	}                                                                                                                                                                                                                                                                                            \
	DLLNETWORK glname##MessageMap *Get##glname##MessageMap() { return g_NetMessages##suffix; }

net_newglobal(Sv, Server, pragma::networking::IServerClient &COMMA NetPacket);
net_newglobal(Cl, Client, NetPacket);

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

void ServerMessageMap::PreRegisterNetMessage(std::string name, void (*)(pragma::networking::IServerClient &, NetPacket))
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

void ServerMessageMap::RegisterNetMessage(std::string name, void (*handler)(pragma::networking::IServerClient &, NetPacket))
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
