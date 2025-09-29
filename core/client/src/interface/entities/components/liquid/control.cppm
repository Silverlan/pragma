// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:entities.components.liquid_control;

import :entities.components.entity;

export namespace pragma {
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
