/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FUNC_PHYSICS_H__
#define __C_FUNC_PHYSICS_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/func/basefuncphysics.h"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma {
	class DLLCLIENT CFuncPhysicsComponent final : public BaseFuncPhysicsComponent, public CBaseNetComponent {
	  public:
		CFuncPhysicsComponent(BaseEntity &ent) : BaseFuncPhysicsComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnEntitySpawn() override;
	};
};

class DLLCLIENT CFuncPhysics : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
