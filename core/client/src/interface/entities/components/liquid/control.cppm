// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.liquid_control;

export import :entities.components.entity;

export namespace pragma {
	namespace cLiquidControlComponent {
		using namespace baseLiquidControlComponent;
	}
	class DLLCLIENT CLiquidControlComponent final : public BaseLiquidControlComponent, public CBaseNetComponent {
	  public:
		CLiquidControlComponent(pragma::ecs::BaseEntity &ent) : BaseLiquidControlComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool OnBulletHit(const game::BulletInfo &bulletInfo, const pragma::physics::TraceData &data, pragma::physics::PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result) override;
	};
};
