// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.triggers.hurt;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER STriggerHurtComponent final : public BaseTriggerHurtComponent {
		public:
			STriggerHurtComponent(BaseEntity &ent) : BaseTriggerHurtComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerHurt : public SBaseEntity {
	protected:
	public:
		virtual void Initialize() override;
	};
};
