// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/components/basetoggle.h>
#include <pragma/entities/trigger/base_trigger_touch.hpp>

export module pragma.server.entities.components.triggers.gravity;

export {
	namespace pragma {
		class DLLSERVER STouchComponent final : public BaseTouchComponent {
		public:
			STouchComponent(BaseEntity &ent) : BaseTouchComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerTouch : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
