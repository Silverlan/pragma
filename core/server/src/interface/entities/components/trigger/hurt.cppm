// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.triggers.hurt;

import :entities;

export {
	namespace pragma {
		class DLLSERVER STriggerHurtComponent final : public BaseTriggerHurtComponent {
		  public:
			STriggerHurtComponent(ecs::BaseEntity &ent) : BaseTriggerHurtComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER TriggerHurt : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
