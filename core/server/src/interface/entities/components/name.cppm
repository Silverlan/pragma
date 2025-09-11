// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/base_name_component.hpp>

export module pragma.server.entities.components.name;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SNameComponent final : public BaseNameComponent, public SBaseNetComponent {
	  public:
		SNameComponent(BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void SetName(std::string name) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
