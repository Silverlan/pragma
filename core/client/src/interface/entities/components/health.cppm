// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_health_component.hpp>

export module pragma.client.entities.components.health;

export namespace pragma {
	class DLLCLIENT CHealthComponent final : public BaseHealthComponent, public CBaseNetComponent {
	  public:
		CHealthComponent(BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
