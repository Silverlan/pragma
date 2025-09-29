// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.entities.components.shooter;

import pragma.entities.components;
import pragma.server.entities.components.entity;

export namespace pragma::ecs {
	class DLLSERVER SShooterComponent final : public BaseShooterComponent, public SBaseNetComponent {
	  public:
		SShooterComponent(BaseEntity &ent) : BaseShooterComponent(ent) {}

		void FireBullets(const BulletInfo &bulletInfo, const std::function<bool(DamageInfo &, BaseEntity *)> &fCallback, std::vector<TraceResult> &outHitTargets, bool bMaster = true);
		virtual void FireBullets(const BulletInfo &bulletInfo, std::vector<TraceResult> &results, bool bMaster = true) override final;
		virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl, pragma::NetEventId, NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override {}
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void FireBullets(const BulletInfo &bulletInfo, DamageInfo &dmgInfo, std::vector<TraceResult> &outHitTargets, const std::function<bool(DamageInfo &, BaseEntity *)> &fCallback = nullptr, bool bMaster = true);
	};
};
