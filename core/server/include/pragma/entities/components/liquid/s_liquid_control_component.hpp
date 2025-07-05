// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LIQUID_CONTROL_COMPONENT_HPP__
#define __S_LIQUID_CONTROL_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_control_component.hpp>

namespace pragma {
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

#endif
