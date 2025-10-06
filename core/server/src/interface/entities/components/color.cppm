// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.entities.components.color;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SColorComponent final : public BaseColorComponent, public SBaseNetComponent {
	  public:
		SColorComponent(BaseEntity &ent) : BaseColorComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
