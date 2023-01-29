/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity_net_event_manager.hpp"

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
