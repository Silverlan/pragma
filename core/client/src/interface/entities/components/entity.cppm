// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include <pragma/entities/entity_component_manager.hpp>

export module pragma.client:entities.components.entity;
export namespace pragma {
	class DLLCLIENT CBaseNetComponent : public BaseNetComponent {
	  public:
		virtual void ReceiveData(NetPacket &packet) = 0;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) { return false; }

		pragma::NetEventId FindNetEvent(const std::string &evName) const;
	};

	class DLLCLIENT CBaseSnapshotComponent : public CBaseNetComponent {
	  public:
		virtual void ReceiveSnapshotData(NetPacket &packet) = 0;
	};

	/////////////////////////////

	class DLLCLIENT CEntityComponentManager : public EntityComponentManager {
	  public:
		static const ComponentId INVALID_COMPONENT;

		const std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable() const;
		std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable();
	  private:
		std::vector<ComponentId> m_svComponentToClComponentTable;
	};
};
