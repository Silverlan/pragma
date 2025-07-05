// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
