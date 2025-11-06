// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.triggers.gravity;

import :entities;

export {
	namespace pragma {
		class DLLSERVER STriggerGravityComponent final : public BaseEntityTriggerGravityComponent {
		  public:
			STriggerGravityComponent(pragma::ecs::BaseEntity &ent) : BaseEntityTriggerGravityComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnStartTouch(pragma::ecs::BaseEntity *ent) override;
			virtual void OnResetGravity(pragma::ecs::BaseEntity *ent, GravitySettings &settings) override;
		};
	};

	class DLLSERVER TriggerGravity : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
