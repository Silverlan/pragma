// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/components/liquid/base_liquid_control_component.hpp>

export module pragma.server.entities.components.liquid_control;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SLiquidControlComponent final : public BaseLiquidControlComponent, public SBaseNetComponent {
	  public:
		SLiquidControlComponent(BaseEntity &ent) : BaseLiquidControlComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;

		virtual void CreateSplash(const Vector3 &origin, float radius, float force) override;
	};
};
