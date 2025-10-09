// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.time_scale;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CTimeScaleComponent final : public BaseTimeScaleComponent, public CBaseNetComponent {
	  public:
		CTimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	};
};
