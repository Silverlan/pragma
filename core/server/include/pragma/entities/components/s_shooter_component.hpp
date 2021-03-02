/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_SHOOTER_COMPONENT_HPP__
#define __S_SHOOTER_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_shooter_component.hpp>

class DamageInfo;
namespace pragma
{
	class DLLSERVER SShooterComponent final
		: public BaseShooterComponent,
		public SBaseNetComponent
	{
	public:
		SShooterComponent(BaseEntity &ent) : BaseShooterComponent(ent) {}

		void FireBullets(const BulletInfo &bulletInfo,const std::function<bool(DamageInfo&,BaseEntity*)> &fCallback,std::vector<TraceResult> &outHitTargets,bool bMaster=true);
		virtual void FireBullets(const BulletInfo &bulletInfo,std::vector<TraceResult> &results,bool bMaster=true) override final;
		virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl,pragma::NetEventId,NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		virtual void FireBullets(const BulletInfo &bulletInfo,DamageInfo &dmgInfo,std::vector<TraceResult> &outHitTargets,const std::function<bool(DamageInfo&,BaseEntity*)> &fCallback=nullptr,bool bMaster=true);
	};
};

#endif
