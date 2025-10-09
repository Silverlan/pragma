// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.score;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CScoreComponent final : public BaseScoreComponent, public CBaseNetComponent {
	  public:
		CScoreComponent(BaseEntity &ent) : BaseScoreComponent(ent) {}

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
