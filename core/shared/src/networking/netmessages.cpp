/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/netmessages.h"

#define net_createmap(suffix, glname)                                                                                                                                                                                                                                                            \
	if(g_NetMessages##suffix == NULL)                                                                                                                                                                                                                                                            \
		g_NetMessages##suffix = new glname##MessageMap;

#define net_newglobal(suffix, glname, args)                                                                                                                                                                                                                                                      \
	DLLNETWORK glname##MessageMap *g_NetMessages##suffix = NULL;                                                                                                                                                                                                                                 \
	DLLNETWORK void Register##glname##NetMessage(std::string name, void (*handler)(args))                                                                                                                                                                                                        \
	{                                                                                                                                                                                                                                                                                            \
		net_createmap(suffix, glname);                                                                                                                                                                                                                                                           \
		if(handler == NULL)                                                                                                                                                                                                                                                                      \
			g_NetMessages##suffix->PreRegisterNetMessage(name, handler);                                                                                                                                                                                                                         \
		else                                                                                                                                                                                                                                                                                     \
			g_NetMessages##suffix->RegisterNetMessage(name, handler);                                                                                                                                                                                                                            \
	}                                                                                                                                                                                                                                                                                            \
	DLLNETWORK glname##MessageMap *Get##glname##MessageMap()                                                                                                                                                                                                                                     \
	{                                                                                                                                                                                                                                                                                            \
		return g_NetMessages##suffix;                                                                                                                                                                                                                                                            \
	}

net_newglobal(Sv, Server, pragma::networking::IServerClient &COMMA NetPacket);
net_newglobal(Cl, Client, NetPacket);

void ClientMessageMap::PreRegisterNetMessage(std::string name, void (*)(NetPacket))
{
	if(m_netMessageIDs.find(name) != m_netMessageIDs.end())
		return;
	CLNetMessage msg;
	msg.ID = m_messageID;
	msg.handler = NULL;
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
	msg.handler = NULL;
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
