// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_POINT_CONSTRAINT_CONETWIST_H__
#define __C_POINT_CONSTRAINT_CONETWIST_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_conetwist.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma {
	class DLLCLIENT CPointConstraintConeTwistComponent final : public BasePointConstraintConeTwistComponent, public CBaseNetComponent {
	  public:
		CPointConstraintConeTwistComponent(BaseEntity &ent) : BasePointConstraintConeTwistComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

class DLLCLIENT CPointConstraintConeTwist : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
