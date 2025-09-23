// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_ownable_component.hpp>
#include <pragma/lua/luaapi.h>

export module pragma.client:entities.components.ownable;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT COwnableComponent final : public BaseOwnableComponent, public CBaseNetComponent {
	  public:
		COwnableComponent(BaseEntity &ent) : BaseOwnableComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	};
};
