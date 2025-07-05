// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LIQUID_CONTROL_COMPONENT_HPP__
#define __C_LIQUID_CONTROL_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_control_component.hpp>

namespace pragma {
	class DLLCLIENT CLiquidControlComponent final : public BaseLiquidControlComponent, public CBaseNetComponent {
	  public:
		CLiquidControlComponent(BaseEntity &ent) : BaseLiquidControlComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool OnBulletHit(const BulletInfo &bulletInfo, const TraceData &data, PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result) override;
	};
};

#endif
