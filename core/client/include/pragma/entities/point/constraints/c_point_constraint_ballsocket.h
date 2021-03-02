/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_POINT_CONSTRAINT_BALLSOCKET_H__
#define __C_POINT_CONSTRAINT_BALLSOCKET_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma
{
	class DLLCLIENT CPointConstraintBallSocketComponent final
		: public BasePointConstraintBallSocketComponent,
		public CBaseNetComponent
	{
	public:
		CPointConstraintBallSocketComponent(BaseEntity &ent) : BasePointConstraintBallSocketComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

class DLLCLIENT CPointConstraintBallSocket
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif