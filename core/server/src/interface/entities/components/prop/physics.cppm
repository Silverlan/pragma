// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server:entities.components.prop.physics;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SPropPhysicsComponent final : public BasePropPhysicsComponent {
		public:
			SPropPhysicsComponent(pragma::ecs::BaseEntity &ent) : BasePropPhysicsComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PropPhysics : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
