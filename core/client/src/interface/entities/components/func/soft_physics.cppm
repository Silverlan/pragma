// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/func/basefuncsoftphysics.hpp>

import pragma.entities.components;

export module pragma.client.entities.components:func_soft_physics;

export {
	namespace pragma {
		class DLLCLIENT CFuncSoftPhysicsComponent final : public BaseFuncSoftPhysicsComponent {
		public:
			CFuncSoftPhysicsComponent(BaseEntity &ent) : BaseFuncSoftPhysicsComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CFuncSoftPhysics : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
}
