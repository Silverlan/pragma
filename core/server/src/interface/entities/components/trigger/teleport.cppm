// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.triggers.teleport;

import :entities;
import :entities.components.triggers.touch;

export {
	namespace pragma {
		class DLLSERVER STriggerTeleportComponent final : public BaseTriggerTeleportComponent {
		  public:
			STriggerTeleportComponent(ecs::BaseEntity &ent) : BaseTriggerTeleportComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER TriggerTeleport : public TriggerTouch {
	  public:
		virtual void Initialize() override;
	};
};
