// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "algorithm"
#include <string>

module pragma.shared;

import :entities.net_event_manager;

using namespace pragma;

bool NetEventManager::FindNetEvent(const std::string &name, NetEventId &outEventId) const
{
	auto it = std::find_if(m_netEventIds.begin(), m_netEventIds.end(), [&name](const std::string &nameOther) { return ustring::compare(nameOther, name, false); });
	if(it == m_netEventIds.end())
		return false;
	outEventId = it - m_netEventIds.begin();
	return true;
}
NetEventId NetEventManager::FindNetEvent(const std::string &name) const
{
	auto r = INVALID_NET_EVENT;
	FindNetEvent(name, r);
	return r;
}
NetEventId NetEventManager::RegisterNetEvent(const std::string &name)
{
	auto r = FindNetEvent(name);
	if(r != INVALID_NET_EVENT)
		return r;
	auto lname = name;
	ustring::to_lower(lname);
	m_netEventIds.push_back(lname);
	return m_netEventIds.size() - 1u;
}

const std::vector<std::string> &NetEventManager::GetNetEventIds() const { return const_cast<NetEventManager *>(this)->GetNetEventIds(); }
std::vector<std::string> &NetEventManager::GetNetEventIds() { return m_netEventIds; }
