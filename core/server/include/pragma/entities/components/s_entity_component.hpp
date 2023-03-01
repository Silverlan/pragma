/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENTITY_COMPONENT_HPP__
#define __S_ENTITY_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/entity_component_manager.hpp>

class NetPacket;
namespace pragma {
	namespace networking {
		class ClientRecipientFilter;
	};
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

#endif
