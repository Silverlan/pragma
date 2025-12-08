// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.shooter;

export import :entities.components.entity;

export namespace pragma::ecs {
	class DLLCLIENT CShooterComponent final : public BaseShooterComponent, public CBaseNetComponent {
	  public:
		CShooterComponent(pragma::ecs::BaseEntity &ent) : BaseShooterComponent(ent) {}

		virtual void FireBullets(const game::BulletInfo &bulletInfo, std::vector<physics::TraceResult> &outHitTargets, bool bMaster = true) override final;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		void FireBullets(const game::BulletInfo &bulletInfo, const Vector3 &origin, const Vector3 &effectsOrigins, const std::vector<Vector3> &destPositions, bool bTransmitToServer, std::vector<physics::TraceResult> &outHitTargets);
	};
};
