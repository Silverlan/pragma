// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.prop_physics;

import :entities.base_entity;
import :entities.components.entity;

export {
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
};
