// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/basetoggle.h>

export module pragma.server.entities.components.toggle;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SToggleComponent final : public BaseToggleComponent, public SBaseNetComponent {
	  public:
		SToggleComponent(BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void TurnOn() override;
		virtual void TurnOff() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
