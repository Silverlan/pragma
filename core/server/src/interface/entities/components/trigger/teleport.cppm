// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.triggers.teleport;

import pragma.server.entities;
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
