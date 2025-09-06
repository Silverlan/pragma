// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <pragma/entities/components/liquid/base_buoyancy_component.hpp>

export module pragma.server.entities.components.liquid_buoyancy;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SBuoyancyComponent final : public BaseBuoyancyComponent, public SBaseNetComponent {
	  public:
		SBuoyancyComponent(BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
