/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PROP_PHYSICS_H__
#define __C_PROP_PHYSICS_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/prop/prop_physics.hpp>

namespace pragma {
	class DLLCLIENT CPropPhysicsComponent final : public BasePropPhysicsComponent {
	  public:
		CPropPhysicsComponent(BaseEntity &ent) : BasePropPhysicsComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLCLIENT CPropPhysics : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
