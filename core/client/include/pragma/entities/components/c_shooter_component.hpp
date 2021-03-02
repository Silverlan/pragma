/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHOOTER_COMPONENT_HPP__
#define __C_SHOOTER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_shooter_component.hpp>

namespace pragma
{
	class DLLCLIENT CShooterComponent final
		: public BaseShooterComponent,
		public CBaseNetComponent
	{
	public:
		CShooterComponent(BaseEntity &ent) : BaseShooterComponent(ent) {}

		virtual void FireBullets(const BulletInfo &bulletInfo,std::vector<TraceResult> &outHitTargets,bool bMaster=true) override final;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		void FireBullets(const BulletInfo &bulletInfo,const Vector3 &origin,const Vector3 &effectsOrigins,const std::vector<Vector3> &destPositions,bool bTransmitToServer,std::vector<TraceResult> &outHitTargets);
	};
};

#endif
