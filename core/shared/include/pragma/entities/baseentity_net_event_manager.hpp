// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASEENTITY_NET_EVENT_MANAGER_HPP__
#define __BASEENTITY_NET_EVENT_MANAGER_HPP__

#include "pragma/networkdefinitions.h"

namespace pragma {
	using NetEventId = uint32_t;
	static const NetEventId INVALID_NET_EVENT = std::numeric_limits<decltype(pragma::INVALID_NET_EVENT)>::max();
	class DLLNETWORK NetEventManager {
	  public:
		NetEventManager() = default;
		bool FindNetEvent(const std::string &name, NetEventId &outEventId) const;
		NetEventId FindNetEvent(const std::string &name) const;
		NetEventId RegisterNetEvent(const std::string &name);

		const std::vector<std::string> &GetNetEventIds() const;
		std::vector<std::string> &GetNetEventIds();
	  private:
		std::vector<std::string> m_netEventIds;
	};
};

#endif
