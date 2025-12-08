// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.server:entities.components.shooter;

import :entities.components.entity;
import pragma.shared;

export namespace pragma::ecs {
	class DLLSERVER SShooterComponent final : public BaseShooterComponent, public SBaseNetComponent {
	  public:
		SShooterComponent(pragma::ecs::BaseEntity &ent) : BaseShooterComponent(ent) {}

		void FireBullets(const game::BulletInfo &bulletInfo, const std::function<bool(game::DamageInfo &, pragma::ecs::BaseEntity *)> &fCallback, std::vector<physics::TraceResult> &outHitTargets, bool bMaster = true);
		virtual void FireBullets(const game::BulletInfo &bulletInfo, std::vector<physics::TraceResult> &results, bool bMaster = true) override final;
		virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl, pragma::NetEventId, NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override {}
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void FireBullets(const game::BulletInfo &bulletInfo, game::DamageInfo &dmgInfo, std::vector<physics::TraceResult> &outHitTargets, const std::function<bool(game::DamageInfo &, pragma::ecs::BaseEntity *)> &fCallback = nullptr, bool bMaster = true);
	};
};
