// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.triggers.push;

import :entities;

export {
	namespace pragma {
		class DLLSERVER STriggerPushComponent final : public BaseTriggerPushComponent {
		  public:
			STriggerPushComponent(ecs::BaseEntity &ent) : BaseTriggerPushComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER TriggerPush : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
