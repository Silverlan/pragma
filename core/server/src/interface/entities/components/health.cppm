// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"

export module pragma.server.entities.components.health;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SHealthComponent final : public BaseHealthComponent, public SBaseNetComponent {
	  public:
		SHealthComponent(BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void SetHealth(unsigned short health) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
