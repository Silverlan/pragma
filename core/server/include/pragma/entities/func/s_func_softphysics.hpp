/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_FUNC_SOFTPHYSICS_HPP__
#define __S_FUNC_SOFTPHYSICS_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/func/s_func_physics.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/func/basefuncsoftphysics.hpp>

namespace pragma {
	class DLLSERVER SFuncSoftPhysicsComponent final : public BaseFuncSoftPhysicsComponent, public SBaseNetComponent {
	  public:
		SFuncSoftPhysicsComponent(BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FuncSoftPhysics : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
