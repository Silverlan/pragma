// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/trigger/base_trigger_teleport.hpp>

export module pragma.server.entities.components.triggers.teleport;

import pragma.server.entities.components.triggers.touch;

export {
	namespace pragma {
		class DLLSERVER STriggerTeleportComponent final : public BaseTriggerTeleportComponent {
		public:
			STriggerTeleportComponent(BaseEntity &ent) : BaseTriggerTeleportComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerTeleport : public TriggerTouch {
	public:
		virtual void Initialize() override;
	};
};
