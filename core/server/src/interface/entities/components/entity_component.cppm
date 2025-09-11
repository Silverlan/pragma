// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/entity_component_manager.hpp>
#include <sharedutils/netpacket.hpp>

export module pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SBaseNetComponent : public BaseNetComponent {
	  public:
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) = 0;
		virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl, pragma::NetEventId, NetPacket &packet) { return false; }
	};

	class DLLSERVER SBaseSnapshotComponent : public SBaseNetComponent {
	  public:
		virtual void SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl) = 0;
		virtual bool ShouldTransmitSnapshotData() const = 0;
	};

	/////////////////////////////

	class DLLSERVER SEntityComponentManager : public EntityComponentManager {
	  public:
		SEntityComponentManager() = default;
	  protected:
		virtual void OnComponentTypeRegistered(const ComponentInfo &componentInfo) override;
	};
};
